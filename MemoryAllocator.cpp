#include "MemoryAllocator.hpp"
#include "utils.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

void MemoryAllocator::printStats(int qc, size_t sizePerProc){

	std::string fileName = "memory_stamp_" + std::to_string(qc) + ".txt";
	std::string path = "TextOutputs/" + fileName;

	std::ofstream MyFile(path);

	std::ostringstream out;
	std::vector<std::pair<std::string, std::pair<size_t, size_t>>> sortedFrag(fragmentations.begin(), fragmentations.end());

	// Sort by starting address descending
	std::sort(sortedFrag.begin(), sortedFrag.end(),
		[](const auto& a, const auto& b) {
			return a.second.first > b.second.first;
		});

	out << "Timestamp: " << getTime() << std::endl;
	out << "Number of processes in memory: " << fragmentations.size() << std::endl;
	out << "Total external fragmentation in KB: " << totalMemorySize-usedMemorySize << std::endl;
	out << "----end---- = " << totalMemorySize << std::endl << std::endl;


	for (const auto& it : sortedFrag) {
		size_t starting = it.second.first;
		size_t actualStarting = starting * sizePerBlock;
		out << actualStarting + sizePerProc << std::endl;
		out << it.first << std::endl;
		out << actualStarting << std::endl;
		out << std::endl;
	}

	out << "----start---- = 0" << std::endl;

	if (MyFile.is_open()) {
		MyFile << out.str() << std::endl;
	}
	MyFile.close();
}

bool MemoryAllocator::allocateMemory(std::string processID, size_t size) {
	if (size == 0 || size > totalMemorySize - usedMemorySize) {
		return false;
	}

	// Compute required blocks
	size_t sizeInBlocks = (size + sizePerBlock - 1) / sizePerBlock;

	for (size_t i = 0; i <= (totalMemorySize / sizePerBlock) - sizeInBlocks; ++i) {
		if (canAllocate(i, i + sizeInBlocks)) {
			// Mark blocks as allocated
			for (size_t j = i; j < i + sizeInBlocks; ++j) {
				memoryBlocks[j] = 0; 
				allocationChecker[j] = false;
			}

			usedMemorySize += size;

			// Compute leftover inside last block
			size_t leftover = (size % sizePerBlock == 0) ? 0 : (sizePerBlock - (size % sizePerBlock));
			fragmentations[processID] = std::make_pair(i, leftover);

			return true;
		}
	}

	return false;
}


void MemoryAllocator::deallocateMemory(std::string processID, size_t size) {
	auto it = fragmentations.find(processID);
	if (it == fragmentations.end()) {
		return;
	}

	size_t startIndex = it->second.first;
	size_t leftover = it->second.second;

	size_t sizeInBlocks = size / sizePerBlock;
	if (size % sizePerBlock != 0) {
		sizeInBlocks++;
	}

	// Free the blocks
	for (size_t j = startIndex; j < startIndex + sizeInBlocks; ++j) {
		memoryBlocks[j] = sizePerBlock;
		allocationChecker[j] = true;
	}

	usedMemorySize -= size;

	fragmentations.erase(it);
}


bool MemoryAllocator::checkProcInMemory(const std::string processID){
	return fragmentations.find(processID) != fragmentations.end();
}
