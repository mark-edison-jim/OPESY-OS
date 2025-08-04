#pragma once

#include <string.h>
#include <string>
#include "ICommand.hpp"
#include <vector>
#include <memory>
#include "utils.hpp"
#include "Process.hpp"
#include <atomic>
#include <deque>
#include <mutex>
#include "screenTerminal.hpp"
#include <unordered_map>
#include "MemoryAllocator.hpp"

class Process {
private:
	int pid;
	std::string name;
	std::vector<std::unique_ptr<ICommand>> commandList;

	std::deque<std::string> logs;

	std::atomic<int> variableCount = 0;

	std::shared_ptr<Screen> screenRef;

	uint64_t commandIndex = 0;

	int cpuCoreID = -1;
	uint64_t totalLines;
	std::string creationDate = getTime();

	std::unordered_map<std::string, std::string> symbolTable = std::unordered_map < std::string, std::string >();

	uint16_t pageSize;
	uint16_t memorySize;

	int numPages;
	std::vector<int> pageToFrame;

	std::string currentAddress = "0x0000";
	std::shared_ptr<MemoryAllocator> memAccRef;

	enum ProcessState {
		READY,
		RUNNING,
		WAITING,
		FINISHED,
		MEMORY_FAULT
	};

	std::atomic<ProcessState> state = READY;

	//Process(int pid, std::string name, RequirementFlags requirements);

	//void addCommand(ICommand::CommandType cmdType);
	//void executeCommand() const;

	//bool isFinished() const;
	//int getRemainingTime() const;

public:
	Process(int pid, const std::string& name, uint64_t totalLines, std::shared_ptr<Screen> screen, uint16_t memorySize, uint16_t sizePerPage, std::shared_ptr<MemoryAllocator> memAcc)
		: pid(pid), name(name), totalLines(totalLines), screenRef(screen), memorySize(memorySize), pageSize(sizePerPage), memAccRef(memAcc) {
		numPages = static_cast<int>(memorySize / sizePerPage);
		numPages = numPages >= 1 ? numPages : 1;
		pageToFrame.resize(numPages, -1);
		memAccRef->createInitialBSPages(numPages, pid);
	}
	void generateRandomCommands();
	void runCommand();
	void beginProcess(int);
	void moveToNextLine();
	uint64_t getCommandIndex() const;
	int getLinesOfCode() const;
	int getPID() const;
	int getCpuCoreID() const;
	std::string getName();
	std::string getDate();
	ProcessState getState() const;
	void handleForInstruction(int, int);
	void handleExplicitForInstruction(int, std::string, int);
	void setWaiting() {
		state = WAITING;
	}
	void setFinished();
	void setInvalidMem();
	std::deque<std::string> getLogs() const {
		//std::lock_guard<std::mutex> logLock(mtx);
		return logs;
	}
	void loadToPhysMem(std::string, uint16_t);
	int checkAccessPhysMem(int pageNumber);
	uint16_t getFromPhysMem(std::string varName);

	void deallocateMemory() {
		symbolTable.clear();
		memAccRef->removeFrames(pageToFrame, pid);
		//memAccRef->removeFromBS(pid);
	}

	std::string incrementHexString(const std::string& hexStr) {
		std::string hexNum = hexStr.substr(2);                // Remove "0x"
		unsigned int num = std::stoul(hexNum, nullptr, 16);  
		num+=2;                                               
		std::stringstream ss;
		ss << "0x" << std::uppercase << std::setfill('0')
			<< std::setw(4) << std::hex << num;                // Format as 0xXXXX
		return ss.str();
	}

	uint16_t getMemorySize() {
		return memorySize;
	}

	auto getSymbolTable() {
		return symbolTable;
	}

	int getSymbolTableSize() {
		return symbolTable.size();
	}

	bool checkForSTSpace() {
		return symbolTable.size() < 32;
	}

	int getVarCount() {
		return variableCount.load();
	}

	void incrementVarCount() {
		variableCount++;
	}

	void commandSwitchCase(ICommand::CommandType, int, int);
	void fixedCommandSet();
	void fixedCommandSet(std::string);
	void fixedSymbols();
	void explicitCommandSwitchCase(ICommand::CommandType type, std::vector<std::string> cmdTokens, int depth);
	uint16_t readFromPhysMem(std::string memaddress);
	void loadToPhysMemAddress(std::string memaddress, uint16_t value);
};