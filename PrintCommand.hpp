#pragma once
#include "ICommand.hpp"
#include <string>
#include <utility>

class PrintCommand : public ICommand {
private:
	std::string text;
	std::pair<uint16_t, uint16_t> getVariable();
	std::string exp_var;
	uint16_t exp_value;

public:
	PrintCommand(int pid, std::string& text, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable);
	void execute(int) override;
	std::string getText() override{
		return text;
	}
	std::string getLog() override {
		return logText;
	}
};