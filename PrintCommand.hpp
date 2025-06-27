#pragma once
#include "ICommand.hpp"
#include <string>
#include <utility>

class PrintCommand : public ICommand {
private:
	std::string text;
	std::pair<uint16_t, uint16_t> getVariable();
	std::string targVar;
	bool explicitDef = false;

public:
	PrintCommand(int pid, std::string& text, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable, bool explicitDef);
	void execute(int) override;
	std::string getText() override{
		return text;
	}
	std::string getLog() override {
		return logText;
	}
	void setExplicit(std::string target) {
		explicitDef = true;
		targVar = target;
	}
};