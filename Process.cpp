#include "Process.hpp"
#include "ICommand.hpp"
#include "PrintCommand.hpp"
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

void Process::generateRandomCommands() {
	for (int i = 0; i < totalLines; ++i) {
		std::string text = "\"Hello World " + std::to_string(i+1) + " from " + getName() + "!\"";
		commandList.push_back(std::make_unique<PrintCommand>(pid, text, symbolTable));
	}
}

void Process::beginProcess(int coreID) {
	cpuCoreID = coreID;
	state = RUNNING;
}

void Process::runCommand(){
	commandList[commandIndex]->execute(cpuCoreID);
	std::string output = "(" + getTime() + ")  Core:" + std::to_string(cpuCoreID) + " \"" + commandList[commandIndex]->getText() + "\"";
	moveToNextLine();
	//std::lock_guard<std::mutex> logLock(*logsMtx);
	if(logs.size() == 5)
		logs.pop_front();
	logs.push_back(output);
	screenRef->update(logs, commandIndex);

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