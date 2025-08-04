#pragma once

#include <Windows.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>	
#include <mutex>
#include "utils.hpp"
#include <algorithm>

class MemoryAllocator {
private:
	std::vector<size_t> memoryBlocks;
	std::vector<bool> allocationChecker; // Maps block index to allocation status
	std::map<std::string, std::pair<int, int>> fragmentations; // Maps process ID to memory blocks
	uint16_t totalMemorySize;
	int numFrames;

	uint16_t sizePerFrame;

	struct Frame {
		int pid;
		int frame;
		size_t used;
		std::vector<uint8_t> values;

		Frame(uint16_t sizePerFrame)
			: pid(-1), frame(-1), used(0), values(sizePerFrame, 0) {
		}
		Frame(int pid, int frame, size_t used, std::vector<uint8_t> values)
			: pid(pid), frame(frame), used(used), values(values) {
		}

		void clear() {
			pid = -1;
			frame = -1;
			used = 0;
			std::fill(values.begin(), values.end(), 0);
		}
	};
	std::vector<Frame> physMem;
	std::mutex physMemMutex;
	int lruPage = 0;

	std::mutex debugPrintMutex;

	std::atomic<size_t> pageIns = 0;
	std::atomic<size_t> pageOuts = 0;

	std::mutex opesyFileMutex;
	std::mutex backstoreMutex;

public:
	MemoryAllocator() = default;

	MemoryAllocator(uint16_t totalSize, uint16_t sizePerFrame) : totalMemorySize(totalSize), sizePerFrame(sizePerFrame), numFrames((int)(totalSize /sizePerFrame)) {
		if (totalSize <= 0) {
			throw std::invalid_argument("Total memory size must be greater than zero.");
		}
		memoryBlocks.resize(totalSize, sizePerFrame);
		allocationChecker.resize(totalSize, true);

		physMem.resize(numFrames, Frame(sizePerFrame));
	}

	~MemoryAllocator() {
		physMem.clear();
		memoryBlocks.clear();
		allocationChecker.clear();
	}

	//bool canAllocate(size_t start, size_t end) {
	//	for (size_t i = start; i < end; ++i) {
	//		if (!allocationChecker[i]) {
	//			return false;
	//		}
	//	}
	//	return true;
	//}


	void DebugPrintFrameList(const std::vector<Frame>& frames) {
		OutputDebugStringA("=== Frame List ===\n");

		int index = 0;
		for (const auto& frame : frames) {
			std::string output;
			output += "Frame " + std::to_string(index++) + ":\n";
			output += "  PID: " + std::to_string(frame.pid) + " ";
			output += "  Page: " + std::to_string(frame.frame) + "\n";
			output += "  Used: " + std::to_string(frame.used) + "\n";
			output += "  Values: ";

			size_t maxDisplay = std::min<size_t>(16, frame.values.size()); // truncate if too large
			for (size_t i = 0; i < maxDisplay; ++i) {
				output += std::to_string(frame.values[i]);
				if (i != maxDisplay - 1) output += ", ";
			}

			if (frame.values.size() > maxDisplay)
				output += ", ... (truncated)";

			output += "\n";
			OutputDebugStringA(output.c_str());
		}

		OutputDebugStringA("===================\n");
	}

	void ForceDebugPrintFrameList(std::string from = "") {
		std::lock_guard<std::mutex> physMemLock(physMemMutex);
		std::lock_guard<std::mutex> debugLock(debugPrintMutex);
		OutputDebugStringA("=== Frame List ===\n");
		int index = 0;
		for (const auto& frame : physMem) {
			std::string output;
			output += "Frame " + std::to_string(index++) + ":\n";
			output += from + "  PID: " + std::to_string(frame.pid) + " ";
			output += "  Page: " + std::to_string(frame.frame) + "\n";
			output += "  Used: " + std::to_string(frame.used) + "\n";
			output += "  Values: ";

			size_t maxDisplay = std::min<size_t>(16, frame.values.size()); // truncate if too large
			for (size_t i = 0; i < maxDisplay; ++i) {
				output += std::to_string(frame.values[i]);
				if (i != maxDisplay - 1) output += ", ";
			}

			if (frame.values.size() > maxDisplay)
				output += ", ... (truncated)";

			output += "\n";
			OutputDebugStringA(output.c_str());
		}

		OutputDebugStringA("===================\n");
	}

	int findLRUPage();

	std::vector<Frame> getPhysMem() {
		std::lock_guard<std::mutex> physMemLock(physMemMutex);
		return physMem;
	}

	int calculateOverallUsedMemory() {
		std::lock_guard<std::mutex> physMemLock(physMemMutex);
		int numFrames = 0;
		for (int i = 0; i < physMem.size(); i++) {
			if (physMem[i].pid >= 0) {
				numFrames++;
			}
		}
		return numFrames;
	}

	int calculatePIDUsedMemory(int pid) {
		std::lock_guard<std::mutex> physMemLock(physMemMutex);
		int numFrames = 0;
		for (int i = 0; i < physMem.size(); i++) {
			if (physMem[i].pid == pid) {
				numFrames++;
			}
		}
		return numFrames;
	}

	void removeFrames(std::vector<int> mapTable, int pid) {
		std::lock_guard<std::mutex> physMemLock(physMemMutex);

		for (int i = 0; i < mapTable.size(); i++) {
			int frameIndex = mapTable[i];
			if (frameIndex >= 0 && physMem[frameIndex].pid == pid) {
				std::string msg = "[RemoveFrames] Clearing frame " + std::to_string(mapTable[i]) +
					" for PID " + std::to_string(pid) + "\n";
				OutputDebugStringA(msg.c_str());
				physMem[mapTable[i]].clear();
			}
		}
	}

	Frame checkFrameAtIndex(int i) {
		std::lock_guard<std::mutex> physMemLock(physMemMutex);
		return physMem[i];
	}

	void backStorePage(int frameIndex);
	void createInitialBSPages(int numPages, int);
	bool findPidInBS(int pid, int pageNumber);
	Frame retrievePageFromBS(int pid, int pageNumber);
	//void removeFromBS(int pid);
	void swapFrameWBS(int pid, int frame, int pageNumber);

	int findPID(int, int);
	int findFreeSpace();

	void assignToFrame(int, std::string, int, uint16_t);
	uint16_t getFromFrame(std::string vma, int frame);

	//void printStats(int qc, size_t sizePerProc);
	//bool allocateMemory(const std::string processID, size_t size, int step);
	//void deallocateMemory(const std::string processID, size_t size, int step);
	//bool checkProcInMemory(const std::string processID);

	size_t getPagedIns() {
		return pageIns.load();
	}

	size_t getPagedOuts() {
		return pageOuts.load();
	}

	int getTotalMemory() const {
		return totalMemorySize;
	}
};