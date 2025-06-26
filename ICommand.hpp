#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <utility>

class ICommand {
public:
	enum CommandType {
		PRINT,
		DECLARE,
		ADD,
		SUBTRACT,
		SLEEP,
		FOR
	};

	ICommand(CommandType type, int pid, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable) : cmdType(type), pid(pid), symbolTable(symbolTable) {}

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
	std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable;

	virtual std::pair<uint16_t, uint16_t> getVariable() = 0;

};
