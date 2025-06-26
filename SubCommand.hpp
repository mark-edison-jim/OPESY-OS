#pragma once
#include "ICommand.hpp"
#include <string>
#include <utility>

class SubCommand : public ICommand {
private:
	std::string text;
	std::pair<uint16_t, uint16_t> getVariable();

	std::string exp_var1;
	uint16_t exp_value1;

	std::string exp_var2;
	uint16_t exp_value2;

	void assignToVar(uint16_t);

public:
	SubCommand(int pid, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable);
	void execute(int) override;
	std::string getText() override {
		return text;
	}
	std::string getLog() override {
		return logText;
	}
};