#define _CRT_SECURE_NO_WARNINGS
#include "MemoryAllocator.hpp"
#include "utils.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <cstring>

int MemoryAllocator::findPID(int pid, int pageNumber) {
	std::lock_guard<std::mutex> physMemLock(physMemMutex);
	DebugPrintFrameList(physMem);

	for (int i = 0; i < physMem.size(); ++i) {
		const Frame& frame = physMem[i];
		if (frame.pid == pid && frame.frame == pageNumber) {
			//int size = frame.values.size();
			//if (size < 32)
			return i;
		}
	}
	return -1;
}

int MemoryAllocator::findFreeSpace() {
	std::lock_guard<std::mutex> physMemLock(physMemMutex);
	for (int i = 0; i < physMem.size(); ++i) {
		Frame frame = physMem[i];
		if (frame.pid < 0)
			return i;
	}
	return -1;
}

bool MemoryAllocator::findPidInBS(int pid, int pageNumber) {
	std::lock_guard<std::mutex> backstoreLock(backstoreMutex);

	std::ifstream inFile("csopesy-backing-store.txt");
	std::string line;

	while (std::getline(inFile, line)) {
		if (line.starts_with("PID:")) {
			std::vector<std::string> split = splitString(line.substr(5), ' ');
			int foundPid = std::stoi(split[0]);  // after "PID: "
			int foundPage = std::stoi(split[2]); // after "Page: "
			OutputDebugStringA("READING foundPid\n");
			OutputDebugStringA(std::to_string(foundPid).c_str());
			OutputDebugStringA("\nREADING foundPage\n");
			OutputDebugStringA(std::to_string(foundPage).c_str());
			OutputDebugStringA("\n");
			if (foundPid == pid && foundPage == pageNumber) {
				return true;
			}
		}
	}
	inFile.close();
	return false;
}

void MemoryAllocator::assignToFrame(int pid, std::string vma, int frame, uint16_t value) {
	std::lock_guard<std::mutex> physMemLock(physMemMutex);
	int offset = hexToInt(vma);
	auto [low, high] = splitToBytes(value);
	if (physMem[frame].pid < 0)
		physMem[frame].pid = pid;
	physMem[frame].values[offset] = low;
	physMem[frame].values[offset + 1] = high;
	physMem[frame].used++;
}

uint16_t MemoryAllocator::getFromFrame(std::string vma, int frame) {
	std::lock_guard<std::mutex> physMemLock(physMemMutex);
	int offset = hexToInt(vma);
	uint8_t low = physMem[frame].values[offset];
	uint8_t high = physMem[frame].values[offset + 1];
	physMem[frame].used++;
	return intoToBytes(low, high);
}

void MemoryAllocator::backStorePage(int frameIndex) {
	std::lock_guard<std::mutex> backstoreLock(backstoreMutex);
	std::lock_guard<std::mutex> physMemLock(physMemMutex);
	//OutputDebugStringA("backStorePage\n");
	//DebugPrintFrameList(physMem);
	Frame& frame = physMem[frameIndex];
	std::ofstream outFile("csopesy-backing-store.txt", std::ios::app);
 
	if (outFile.is_open()) {
		std::string pidLine = "PID: " + std::to_string(frame.pid) +
			" Page: " + std::to_string(frame.frame) + "\n";
		OutputDebugStringA("WRITING Backstore\n");
		OutputDebugStringA(pidLine.c_str());
		outFile << pidLine;
		outFile << "Used: " << frame.used << "\n";

		for (uint8_t byte : frame.values) {
			outFile << std::uppercase << std::hex
				<< std::setw(2) << std::setfill('0')
				<< static_cast<int>(byte) << " ";
		}
		outFile << "\n";  
	}
	outFile.close();

	physMem[frameIndex] = Frame(sizePerFrame);
	pageOuts++;
}

void MemoryAllocator::createInitialBSPages(int numPages, int pid) {
	std::lock_guard<std::mutex> backstoreLock(backstoreMutex);
	std::lock_guard<std::mutex> physMemLock(physMemMutex);
	//OutputDebugStringA("backStorePage\n");
	//DebugPrintFrameList(physMem);

	std::ofstream outFile("csopesy-backing-store.txt", std::ios::app);

	for (int i = 0; i < numPages; i++) {
		Frame frame = Frame(sizePerFrame);
		frame.pid = pid;
		frame.frame = i;
		if (outFile.is_open()) {
			std::string pidLine = "PID: " + std::to_string(frame.pid) +
				" Page: " + std::to_string(frame.frame) + "\n";
			OutputDebugStringA("WRITING Create\n");
			OutputDebugStringA(pidLine.c_str());
			outFile << pidLine;
			outFile << "Used: " << frame.used << "\n";

			for (uint8_t byte : frame.values) {
				outFile << std::uppercase << std::hex
					<< std::setw(2) << std::setfill('0')
					<< static_cast<int>(byte) << " ";
			}
			outFile << "\n";
		}
	}

	outFile.close();
}

MemoryAllocator::Frame MemoryAllocator::retrievePageFromBS(int pid, int pageNumber) {
	std::lock_guard<std::mutex> backstoreLock(backstoreMutex);
	std::lock_guard<std::mutex> physMemLock(physMemMutex);

	std::ifstream inFile("csopesy-backing-store.txt");
	std::ofstream tempFile("backstoreTemp.txt");

	std::string pidLine, usedLine, valuesLine;
	Frame result(sizePerFrame);
	bool found = false;

	while (std::getline(inFile, pidLine)) {
		std::getline(inFile, usedLine);
		std::getline(inFile, valuesLine);

		if (!found && pidLine.rfind("PID:", 0) == 0) {
			std::vector<std::string> split = splitString(pidLine.substr(5), ' ');
			int foundPid = std::stoi(split[0]);  // after "PID: "
			int foundPage = std::stoi(split[2]); // after "Page: "

			std::string debugMsg = "[DEBUG] Found PID: " + std::to_string(foundPid) + " Page: " + std::to_string(foundPage) + "\n";
			OutputDebugStringA(debugMsg.c_str());

			if (foundPid == pid && foundPage == pageNumber) {
				int used = std::stoi(usedLine.substr(6));  // after "Used: "
				OutputDebugStringA("RETRIVING foundPid\n");
				OutputDebugStringA(std::to_string(foundPid).c_str());
				OutputDebugStringA("\RETRIVING foundPage\n");
				OutputDebugStringA(std::to_string(foundPage).c_str());
				OutputDebugStringA("\n");
				std::vector<uint8_t> values;
				std::istringstream valueStream(valuesLine);
				std::string token;
				while (valueStream >> token) {
					int byte;
					std::istringstream(token) >> std::hex >> byte;
					values.push_back(static_cast<uint8_t>(byte));
				}

				result = Frame(pid, foundPage, used, values);
				found = true;
				continue;  // Skip writing this 3-line block
			}
		}

		// Write this block to temp file
		tempFile << pidLine << "\n";
		tempFile << usedLine << "\n";
		tempFile << valuesLine << "\n";
	}

	// Flush and close properly before rename
	inFile.close();
	tempFile.flush();
	tempFile.close();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	// Remove original and rename with retry
	if (std::ifstream("csopesy-backing-store.txt")) {
		OutputDebugStringA("[DEBUG] File still exists before remove.\n");
	}
	int removeResult = std::remove("csopesy-backing-store.txt");
	if (removeResult != 0) {
		std::string msg = "[DEBUG] Remove failed with errno=" + std::to_string(errno) + "\n";
		OutputDebugStringA(msg.c_str());
	}

	bool renamed = false;
	int retries = 0;
	while (!renamed) {
		if (std::rename("backstoreTemp.txt", "csopesy-backing-store.txt") == 0) {
			OutputDebugStringA("[DEBUG] Rename successful.\n");
			renamed = true;
			break;
		}

		std::string retryString = "[DEBUG] Rename failed (errno=" + std::to_string(errno) + "): " + std::strerror(errno) +
			". Retrying... Attempt #" + std::to_string(retries) + "\n";
		OutputDebugStringA(retryString.c_str());

		retries++;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	if (retries > 0) {
		std::string retryString = "[DEBUG] Rename failed, retrying... Attempt #" + std::to_string(retries) + "\n";
		OutputDebugStringA(retryString.c_str());
	}

	if (found) 
		pageIns++;
	return result;
}

void MemoryAllocator::removeFromBS(int pid) {
	std::lock_guard<std::mutex> backstoreLock(backstoreMutex);
	std::lock_guard<std::mutex> physMemLock(physMemMutex);

	std::ifstream inFile("csopesy-backing-store.txt");
	std::ofstream tempFile("backstoreTemp.txt");

	std::string pidLine, usedLine, valuesLine;
	bool found = false;

	while (std::getline(inFile, pidLine)) {
		std::getline(inFile, usedLine);
		std::getline(inFile, valuesLine);

		if (!found && pidLine.rfind("PID:", 0) == 0) {
			std::vector<std::string> split = splitString(pidLine.substr(5), ' ');
			int foundPid = std::stoi(split[0]);  // after "PID: "
			std::string debugMsg = "[DEBUG] Found PID to remove: " + std::to_string(foundPid) + "\n";
			OutputDebugStringA(debugMsg.c_str());

			if (foundPid == pid) {
				found = true;
				continue;
			}
		}

		tempFile << pidLine << "\n";
		tempFile << usedLine << "\n";
		tempFile << valuesLine << "\n";
	}

	inFile.close();
	tempFile.flush();
	tempFile.close();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	std::remove("csopesy-backing-store.txt");

	bool renamed = false;
	int retries = 0;
	while (!renamed) {
		if (std::rename("backstoreTemp.txt", "csopesy-backing-store.txt") == 0) {
			OutputDebugStringA("[DEBUG] Rename successful.\n");
			break;
		}
		retries++;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	if (retries > 0) {
		std::string retryString = "[DEBUG] Rename failed, retrying... Attempt #" + std::to_string(retries) + "\n";
		OutputDebugStringA(retryString.c_str());
	}

}


//void MemoryAllocator::printStats(int qc, size_t sizePerProc){
//
//	std::string fileName = "memory_stamp_" + std::to_string(qc) + ".txt";
//	std::string path = "TextOutputs/" + fileName;
//
//	std::ofstream MyFile(path);
//
//	std::ostringstream out;
//	std::vector<std::pair<std::string, std::pair<size_t, size_t>>> sortedFrag(fragmentations.begin(), fragmentations.end());
//
//	// Sort by starting address descending
//	std::sort(sortedFrag.begin(), sortedFrag.end(),
//		[](const auto& a, const auto& b) {
//			return a.second.first > b.second.first;
//		});
//
//	out << "Timestamp: " << getTime() << std::endl;
//	out << "Number of processes in memory: " << fragmentations.size() << std::endl;
//	out << "Total external fragmentation in KB: " << totalMemorySize-usedMemorySize << std::endl;
//	out << "----end---- = " << totalMemorySize << std::endl << std::endl;
//
//
//	for (const auto& it : sortedFrag) {
//		size_t starting = it.second.first;
//		size_t actualStarting = starting * sizePerFrame;
//		out << actualStarting + sizePerProc << std::endl;
//		out << it.first << std::endl;
//		out << actualStarting << std::endl;
//		out << std::endl;
//	}
//
//	out << "----start---- = 0" << std::endl;
//
//	if (MyFile.is_open()) {
//		MyFile << out.str() << std::endl;
//	}
//	MyFile.close();
//}
//bool MemoryAllocator::allocateMemory(std::string processID, size_t size, int step) {
//	if (size == 0 || size > totalMemorySize - usedMemorySize)
//		return false;
//
//	// Compute required blocks
//	size_t sizeInBlocks = (size + sizePerFrame - 1) / sizePerFrame;
//
//	for (size_t i = 0; i <= (totalMemorySize / sizePerFrame) - sizeInBlocks; ++i) {
//		if (canAllocate(i, i + sizeInBlocks)) {
//			// Mark blocks as allocated
//			for (size_t j = i; j < i + sizeInBlocks; ++j) {
//				memoryBlocks[j] = 0;
//				allocationChecker[j] = false;
//			}
//
//			usedMemorySize += size;
//
//			// Compute leftover inside last block
//			size_t leftover = (size % sizePerFrame == 0) ? 0 : (sizePerFrame - (size % sizePerFrame));
//			fragmentations[processID] = std::make_pair(i, leftover);
//			//printStats(step, size);
//			return true;
//		}
//	}
//
//	return false;
//}
//
//
//void MemoryAllocator::deallocateMemory(std::string processID, size_t size, int step) {
//	auto it = fragmentations.find(processID);
//	if (it == fragmentations.end()) {
//		return;
//	}
//	//printStats(step, size);
//	size_t startIndex = it->second.first;
//	size_t leftover = it->second.second;
//
//	size_t sizeInBlocks = size / sizePerFrame;
//	if (size % sizePerFrame != 0) {
//		sizeInBlocks++;
//	}
//
//	// Free the blocks
//	for (size_t j = startIndex; j < startIndex + sizeInBlocks; ++j) {
//		memoryBlocks[j] = sizePerFrame;
//		allocationChecker[j] = true;
//	}
//
//	usedMemorySize -= size;
//
//	fragmentations.erase(it);
//}
//
//
//bool MemoryAllocator::checkProcInMemory(const std::string processID){
//	return fragmentations.find(processID) != fragmentations.end();
//}
