#pragma once

#include <string>
#include <unordered_map>
#include <memory>

class ICommand {
public:
	enum CommandType {
		PRINT,
	};

	ICommand(CommandType type, int pid, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable) : cmdType(type), pid(pid), symbolTable(symbolTable) {}

	CommandType getCommandType() const {
		return cmdType;
	}

	virtual std::string getText() = 0;
	virtual void execute(int) = 0;

protected:
	int pid;
	CommandType cmdType;
	std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable;
};
