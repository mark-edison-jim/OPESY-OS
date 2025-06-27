#pragma once
#include "ICommand.hpp"
#include <string>
#include <utility>

class SubCommand : public ICommand {
private:
	std::string text;
	std::pair<uint16_t, uint16_t> getVariable();

	std::string targVar;

	std::string exp_var1;
	uint16_t exp_value1;

	std::string exp_var2;
	uint16_t exp_value2;
	bool explicitDef = false;

	void computeExplicitValues();

	void assignToVar(uint16_t);

public:
	SubCommand(int pid, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable, bool explicitDef);
	void execute(int) override;
	std::string getText() override {
		return text;
	}
	std::string getLog() override {
		return logText;
	}
	void setExplicit(std::string target, std::string var1, uint16_t value1, std::string var2, uint16_t value2) {
		explicitDef = true;
		targVar = target;
		exp_var1 = var1;
		exp_value1 = value1;
		exp_var2 = var2;
		exp_value2 = value2;
	}
};