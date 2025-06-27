#include "Process.hpp"
#include "ICommand.hpp"
#include "PrintCommand.hpp"
#include "DeclareCommand.hpp"
#include "AddCommand.hpp"
#include "SubCommand.hpp"
#include "SleepCommand.hpp"
#include <chrono>
#include <thread> 

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
		commandList.push_back(std::make_unique<PrintCommand>(pid, text, symbolTable, false));
		break;
	}
	case ICommand::DECLARE: {
		commandList.push_back(std::make_unique<DeclareCommand>(pid, symbolTable, false));
		break;
	}
	case ICommand::ADD: {
		commandList.push_back(std::make_unique<AddCommand>(pid, symbolTable, false));
		break;
	}
	case ICommand::SUBTRACT: {
		commandList.push_back(std::make_unique<SubCommand>(pid, symbolTable, false));
		break;
	}
	case ICommand::SLEEP: {
		commandList.push_back(std::make_unique<SleepCommand>(pid, symbolTable, false));
		break;
	}
	case ICommand::FOR: {
		handleForInstruction(getRandomFromRange(remainingIns), depth);
		break;
	}
	default:
		break;
	}
}

void Process::fixedSymbols() {
	std::vector<std::string> varNames{ "x", "y", "z" };
	for (int i = 0; i < 3; i++) {

		symbolTable->insert({ varNames[i], 0 });
	}
}

void Process::fixedCommandSet() {
	std::vector<std::string> varNames{ "x", "y", "z" };
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 3; j++) {
			auto add = std::make_unique<AddCommand>(pid, symbolTable, false);
			add->setExplicit(varNames[j], varNames[j], 0, "", 1);
			commandList.push_back(std::move(add));

			std::string text = "";
			auto print = std::make_unique<PrintCommand>(pid, text, symbolTable, false);
			print->setExplicit(varNames[j]);
			commandList.push_back(std::move(print));
		}
	}
}

void Process::generateRandomCommands() {
	while (commandList.size() < totalLines) {
		int remainingIns = totalLines - commandList.size();
		if (remainingIns <= 0) break;
		ICommand::CommandType type = static_cast<ICommand::CommandType>(getRandomFromRange(6));
		commandSwitchCase(type, remainingIns, 3);
	}	
}

void Process::handleForInstruction(int loops, int depth) {
	if (depth <= 0 || loops <= 0 || commandList.size() >= totalLines) return;
	for (int i = 0; i < loops; ++i) {
		int remainingIns = totalLines - commandList.size();
		if (remainingIns <= 0) break;
		ICommand::CommandType type = static_cast<ICommand::CommandType>(getRandomFromRange(6));
		commandSwitchCase(type, remainingIns, depth-1);
	}
}

void Process::beginProcess(int coreID) {
	cpuCoreID = coreID;
	state = RUNNING;
}

void Process::runCommand(){
	commandList[commandIndex]->execute(cpuCoreID);

	//std::lock_guard<std::mutex> logLock(*logsMtx);
	//if(logs.size() == 5)
		//logs.pop_front();

	if(commandList[commandIndex]->getCommandType() == ICommand::PRINT){
		std::string output = "(" + getTime() + ")  Core:" + std::to_string(cpuCoreID) + " \"" + commandList[commandIndex]->getLog() + "\"";
		logs.push_back(output);
		screenRef->update(logs, commandIndex+1);
		//screenRef->addCommand("", commandList[commandIndex]->getLog());
	}

	if (commandList[commandIndex]->getCommandType() == ICommand::SLEEP) {
		SleepCommand* inst = dynamic_cast<SleepCommand*>(commandList[commandIndex].get());
		uint8_t sleepTime = inst->getSleepTime();
		if (sleepTime <= 0)
			moveToNextLine();
	}else {
		moveToNextLine();
	}


	if (commandIndex >= totalLines) {
		setFinished();
		screenRef->finish(logs);
		//if(coreID == 2)
		//break;
	}
}


int Process::getCommandIndex() const
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