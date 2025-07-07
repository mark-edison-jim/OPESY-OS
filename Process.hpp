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

class Process {
private:
	int pid;
	std::string name;
	std::vector<std::unique_ptr<ICommand>> commandList;
	//std::shared_ptr<std::mutex> logsMtx;
	std::deque<std::string> logs;

	std::shared_ptr<Screen> screenRef;

	size_t memorySize;

	uint64_t commandIndex = 0;

	int cpuCoreID = -1;
	uint64_t totalLines;
	std::string creationDate = getTime();

	std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable = std::make_shared<std::unordered_map<std::string, uint16_t>>();

	//struct RequirementFlags {
	//	bool requireFiles;
	//	int numFiles;
	//	bool requireMemory;
	//	int memoryRequired;
	//};

	enum ProcessState {
		READY,
		RUNNING,
		WAITING,
		FINISHED
	};
	std::atomic<ProcessState> state = READY;

	//Process(int pid, std::string name, RequirementFlags requirements);

	//void addCommand(ICommand::CommandType cmdType);
	//void executeCommand() const;

	//bool isFinished() const;
	//int getRemainingTime() const;




public:
	Process(int pid, const std::string& name, uint64_t totalLines, std::shared_ptr<Screen> screen, size_t memorySize)
		: pid(pid), name(name), totalLines(totalLines), screenRef(screen), memorySize(memorySize){
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
	void setWaiting() {
		state = WAITING;
	}
	void setFinished();
	std::deque<std::string> getLogs() const {
		//std::lock_guard<std::mutex> logLock(mtx);
		return logs;
	}

	size_t getMemorySize() {
		return memorySize;
	}

	void commandSwitchCase(ICommand::CommandType, int, int);
	void fixedCommandSet();
	void fixedSymbols();
	//RequirementFlags requirements;
};