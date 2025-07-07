#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>

class MemoryAllocator {
private:
	std::vector<size_t> memoryBlocks;
	std::vector<bool> allocationChecker; // Maps block index to allocation status
	std::map<std::string, std::pair<int, int>> fragmentations; // Maps process ID to memory blocks
	size_t totalMemorySize;
	size_t sizePerBlock;
	size_t usedMemorySize = 0;

public:
	MemoryAllocator() = default;

	MemoryAllocator(size_t totalSize, size_t sizePerBlock) : totalMemorySize(totalSize), sizePerBlock(sizePerBlock) {
		if (totalSize <= 0) {
			throw std::invalid_argument("Total memory size must be greater than zero.");
		}
		memoryBlocks.resize(totalSize, sizePerBlock);
		allocationChecker.resize(totalSize, true);
	}

	~MemoryAllocator() {
		memoryBlocks.clear();
		allocationChecker.clear();
	}

	bool canAllocate(size_t start, size_t end) {
		for (size_t i = start; i < end; ++i) {
			if (!allocationChecker[i]) {
				return false;
			}
		}
		return true;
	}

	void printStats(int qc, size_t sizePerProc);
	bool allocateMemory(const std::string processID, size_t size);
	void deallocateMemory(const std::string processID, size_t size);
	bool checkProcInMemory(const std::string processID);

	int getUsedMemory() const {
		return usedMemorySize;
	}

	int getTotalMemory() const {
		return totalMemorySize;
	}
};