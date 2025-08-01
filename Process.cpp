#include <windows.h>
#include "Process.hpp"
#include "ICommand.hpp"
#include "PrintCommand.hpp"
#include "DeclareCommand.hpp"
#include "AddCommand.hpp"
#include "SubCommand.hpp"
#include "SleepCommand.hpp"
#include <chrono>
#include <thread> 
#include <debugapi.h>

std::string Process::getName() {
	return name;
}

std::string Process::getDate()
{
	return creationDate;
}

Process::ProcessState Process::getState() const
{
	return state.load();
}

void Process::setFinished()
{
	state = FINISHED;
}

void Process::moveToNextLine()
{
	commandIndex++;
}

void Process::commandSwitchCase(ICommand::CommandType type, int remainingIns, int depth) {
	switch (type)
	{
	case ICommand::PRINT: {
		std::string text = "\"Hello World from <" + getName() + ">!\"";
		commandList.push_back(std::make_unique<PrintCommand>(pid, text, false, this));
		break;
	}
	case ICommand::DECLARE: {
		commandList.push_back(std::make_unique<DeclareCommand>(pid, false, this));
		break;
	}
	case ICommand::ADD: {
		commandList.push_back(std::make_unique<AddCommand>(pid, false, this));
		break;
	}
	case ICommand::SUBTRACT: {
		commandList.push_back(std::make_unique<SubCommand>(pid, false, this));
		break;
	}
	case ICommand::SLEEP: {
		commandList.push_back(std::make_unique<SleepCommand>(pid, false, this));
		break;
	}
	case ICommand::FOR: {
		handleForInstruction(getRandomFromRange(0, remainingIns), depth);
		break;
	}
	default:
		break;
	}
}

void Process::fixedSymbols() {
	std::vector<std::string> varNames{ "x" };
	for (int i = 0; i < varNames.size(); i++) {
		loadToPhysMem(varNames[i], 0);
		//symbolTable->insert({ varNames[i], 0 });
	}
}

void Process::fixedCommandSet() {
	std::vector<std::string> varNames{ "x" };
	for (int i = 0; i < totalLines; i++) {
		for (int j = 0; j < varNames.size(); j++) {
			std::string text = "";
			auto print = std::make_unique<PrintCommand>(pid, text, false, this);
			print->setExplicit(varNames[j]);
			commandList.push_back(std::move(print));

			auto add = std::make_unique<AddCommand>(pid, false, this);
			add->setExplicit(varNames[j], varNames[j], 0, "", getRandomFromRange(1, 10));
			commandList.push_back(std::move(add));
		}
	}
}

void Process::generateRandomCommands() {
	while (commandList.size() < totalLines) {
		int remainingIns = totalLines - commandList.size();
		if (remainingIns <= 0) break;
		ICommand::CommandType type = static_cast<ICommand::CommandType>(getRandomFromRange(0, 6));
		commandSwitchCase(type, remainingIns, 3);
	}	
}

void Process::handleForInstruction(int loops, int depth) {
	if (depth <= 0 || loops <= 0 || commandList.size() >= totalLines) return;
	for (int i = 0; i < loops; ++i) {
		int remainingIns = totalLines - commandList.size();
		if (remainingIns <= 0) break;
		ICommand::CommandType type = static_cast<ICommand::CommandType>(getRandomFromRange(0, 6));
		commandSwitchCase(type, remainingIns, depth-1);
	}
}

void Process::beginProcess(int coreID) {
	cpuCoreID = coreID;
	state = RUNNING;
}

void Process::runCommand(){
	commandList[commandIndex]->execute(cpuCoreID);

	if(commandList[commandIndex]->getCommandType() == ICommand::PRINT){
		std::string output = "(" + getTime() + ")  Core:" + std::to_string(cpuCoreID) + " \"" + commandList[commandIndex]->getLog() + "\"";
		if(logs.size() == 10)
			logs.pop_front();
		logs.push_back(output);
		screenRef->update(logs, commandIndex+1);
		//screenRef->addCommand("", commandList[commandIndex]->getLog());
	}

	if (commandList[commandIndex]->getCommandType() == ICommand::SLEEP) {
		if (SleepCommand* inst = dynamic_cast<SleepCommand*>(commandList[commandIndex].get())) {
			uint8_t sleepTime = inst->getSleepTime();
			if (sleepTime <= 0)
				moveToNextLine();
		}
		else {
			moveToNextLine();
		}
	}
	else {
		moveToNextLine();
	}
		
	if (commandIndex >= totalLines) {
		setFinished();
		screenRef->finish(logs);
	}
}

void DebugPrintSymbolTable(const std::unordered_map<std::string, std::string>& symbolTable) {
	OutputDebugStringA("\nTest\n");
	for (const auto& pair : symbolTable) {
		std::string line = pair.first + " : " + pair.second + "\n";
		OutputDebugStringA(line.c_str());
	}
	//OutputDebugStringA("\nVMA: ");
}

inline const char* CommandTypeToString(ICommand::CommandType type) {
	switch (type) {
	case ICommand::PRINT:    return "PRINT";
	case ICommand::DECLARE:  return "DECLARE";
	case ICommand::ADD:      return "ADD";
	case ICommand::SUBTRACT: return "SUBTRACT";
	case ICommand::SLEEP:    return "SLEEP";
	case ICommand::FOR:      return "FOR";
	default:                 return "UNKNOWN";
	}
}


void DebugPrintCMDList(std::vector<std::unique_ptr<ICommand>>& list) {
	OutputDebugStringA("Command List:\n");
	for (const auto& cmd : list) {
		std::string line = std::string(CommandTypeToString(cmd->getCommandType())) + "\n";
		OutputDebugStringA(line.c_str());
	}
}

void DebugPrintMap(std::vector<int>& list) {
	OutputDebugStringA("P2F List of:\n");
	int i = 0;
	for (const auto& frame : list) {
		std::string line = std::to_string(i) + " : " + std::to_string(frame) + "\n";
		OutputDebugStringA(line.c_str());
		i++;
	}
}

void Process::loadToPhysMem(std::string varName, uint16_t value){
	
	int intHex = hexToInt(currentAddress);
	int pageNumber = intHex / pageSize;
	std::string EdebugStr = std::string("\n LOAD BEFORE CHECK: ") + std::to_string(pid) + std::string(" Type: ") + CommandTypeToString(commandList[commandIndex]->getCommandType())
		+ std::string(" Instruction Count: ") + std::to_string(commandIndex) + "\n";
	OutputDebugStringA(EdebugStr.c_str());
	memAccRef->ForceDebugPrintFrameList("LOAD BEFORE CHECK");

	int frameNum = checkAccessPhysMem(pageNumber);

	//if (pageToFrame[pageNumber] == -1)
	pageToFrame[pageNumber] = frameNum;
	if (symbolTable.find(varName) == symbolTable.end()) {
		symbolTable[varName] = currentAddress;
		currentAddress = incrementHexString(currentAddress);
	}
	std::string debugStr = std::string("\n LOAD Process: ") + std::to_string(pid) + std::string(" Type: ") + CommandTypeToString(commandList[commandIndex]->getCommandType())
		+ std::string(" Instruction Count: ") + std::to_string(commandIndex) + "\n";
	OutputDebugStringA(debugStr.c_str());
	memAccRef->ForceDebugPrintFrameList("LOAD Process:");
	//DebugPrintSymbolTable(symbolTable);

	//TODO: FIGURE OUT -1 PROBLEM IN FRAME.FRAME

	memAccRef->assignToFrame(pid, symbolTable[varName], pageToFrame[pageNumber], value);
	OutputDebugStringA("AFTER assignToFrame: \n");
	memAccRef->ForceDebugPrintFrameList("assignToFrame");
}

	//TODO: There might be problem with swap func, like free space or kicking out, in fcfs, never need to swap

int Process::checkAccessPhysMem(int pageNumber) {
	//1. find pid
	//2. if pid exists: use that pid frame
	//3. if no pid: check free frame
	//4.	yes ff : check backing for pid
	//5.				yes backing : load backing page into free space
	//6. 				no backing  : use freespace as new page
	//7.	no ff  : check backing for pid
	//8.				yes backing : swap with lru
	//9.				no backing  : kick out lru and use that space

	//DebugPrintMap(pageToFrame);
	int pidFrameNum = memAccRef->findPID(pid, pageNumber); // find pid
	if (pidFrameNum < 0) { // No Existing PID Found
		int freeFrameNum = memAccRef->findFreeSpace();
		OutputDebugStringA("[DEBUG] Free frame chosen: ");
		OutputDebugStringA(std::to_string(freeFrameNum).c_str());
		OutputDebugStringA("\n");
		if (freeFrameNum < 0) { //no free space and no existing PID in physmem
			int lru = {};
			int test = 0;
			if (bool found = memAccRef->findPidInBS(pid, pageNumber)) {// swap frame and bs
				int lru = memAccRef->findLRUPage();
				memAccRef->swapFrameWBS(pid, lru, pageNumber);

				OutputDebugStringA("BAD Part: ");
				memAccRef->ForceDebugPrintFrameList();
				return lru;
			}else {
				OutputDebugStringA("NOOB Part: ");
			}
			//else {// kick out lru and use that space
			//	int lru = memAccRef->findLRUPage();
			//	//memAccRef->backStorePage(lru);
			//	memAccRef->swapFrameWBS(pid, lru, pageNumber);
			//	OutputDebugStringA("WORSE Part: ");
			//	test = 1;
			//}
			//if (memAccRef->checkFrameAtIndex(lru).pid == -1 && memAccRef->checkFrameAtIndex(lru).frame == -1) {
			//	return test;
			//}
		}
		else {// has free space but no existing PID in physmem
			if (memAccRef->findPidInBS(pid, pageNumber)) { // load backing page into free space
				memAccRef->swapFrameWBS(pid, freeFrameNum, pageNumber);
				if (freeFrameNum == -1)
					std::cout << "hi\n";
				return freeFrameNum;
			}
			else {
				std::cout << "hi\n";
			}
			//if (memAccRef->checkFrameAtIndex(freeFrameNum).pid == -1) {
			//	return -1;
			//}
			 //use freespace as new page
		}
	}
	//if (memAccRef->checkFrameAtIndex(pidFrameNum).pid == -1) {
	//	return -1;
	//}
	if (pidFrameNum == -1)
		std::cout << "hi\n";
	return pidFrameNum; //if pid exists: use that pid frame
}

uint16_t Process::getFromPhysMem(std::string varName) {
	std::string vma = symbolTable[varName];
	//DebugPrintSymbolTable(symbolTable);
	//OutputDebugStringA(varName.c_str());
	int pageNumber = hexToInt(vma) / pageSize;
	std::string EdebugStr = std::string("\n GET BEFORE CHECK: ") + std::to_string(pid) + std::string(" Type: ") + CommandTypeToString(commandList[commandIndex]->getCommandType())
		+ std::string(" Instruction Count: ") + std::to_string(commandIndex) + "\n";
	OutputDebugStringA(EdebugStr.c_str());
	memAccRef->ForceDebugPrintFrameList("GET BEFORE CHECK=");

	int frameNum = checkAccessPhysMem(pageNumber);

	pageToFrame[pageNumber] = frameNum;
	std::string debugStr = std::string("\n GET Process: ") + std::to_string(pid) + std::string(" Type: ") + CommandTypeToString(commandList[commandIndex]->getCommandType())
		+ std::string(" Instruction Count: ") + std::to_string(commandIndex) + "\n";
	OutputDebugStringA(debugStr.c_str());
	memAccRef->ForceDebugPrintFrameList("GET Process:");
	//DebugPrintSymbolTable(symbolTable);
	//OutputDebugStringA("getFromPhysMem\n");
	//DebugPrintSymbolTable(symbolTable);
	//DebugPrintMap(pageToFrame);
	uint16_t value = memAccRef->getFromFrame(vma, pageToFrame[pageNumber]);

	OutputDebugStringA("AFTER getFromFrame: \n");
	memAccRef->ForceDebugPrintFrameList("getFromFrame");
	return value;
}

uint64_t Process::getCommandIndex() const
{
	return commandIndex;
}

int Process::getLinesOfCode() const
{
	return totalLines;
}

int Process::getPID() const
{
	return pid;
}

int Process::getCpuCoreID() const
{
	return cpuCoreID;
}