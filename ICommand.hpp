#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <utility>
//#include "Process.hpp"

class Process;

class ICommand {
public:
	enum CommandType {
		PRINT,
		DECLARE,
		ADD,
		SUBTRACT,
		SLEEP,
		FOR,
		READ,
		WRITE
	};

	ICommand(CommandType type, int pid, Process* processRef) : cmdType(type), pid(pid), processRef(processRef) {}

	CommandType getCommandType() const {
		return cmdType;
	}

	virtual std::string getText() = 0;
	virtual void execute(int) = 0;
	virtual std::string getLog() = 0;

protected:
	int pid;
	CommandType cmdType;
	std::string logText;
	Process* processRef;

	virtual std::pair<uint16_t, uint16_t> getVariable() = 0;

};
