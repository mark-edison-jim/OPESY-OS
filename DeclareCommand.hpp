#pragma once
#include "ICommand.hpp"
#include <string>
#include <utility>

class DeclareCommand : public ICommand {
private:
	std::string text;

	std::string targVar;
	uint16_t exp_value;

	bool explicitDef = false;
	void declareExplicitVar();
	void assignToVar(uint16_t);
	std::pair<uint16_t, uint16_t> getVariable();

public:
	DeclareCommand(int pid, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable, bool explicitDef);
	void execute(int) override;
	std::string getText() override {
		return text;
	}
	std::string getLog() override {
		return logText;
	}
	void setExplicit(std::string target, uint16_t value) {
		explicitDef = true;
		targVar = target;
		exp_value = value;
	}
};