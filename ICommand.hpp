#pragma once

#include <string>

class ICommand {
public:
	enum CommandType {
		PRINT,
	};

	ICommand(CommandType type, int pid) : cmdType(type), pid(pid) {}

	CommandType getCommandType() const {
		return cmdType;
	}

	virtual std::string getText() = 0;
	virtual void execute() = 0;

protected:
	int pid;
	CommandType cmdType;

};
