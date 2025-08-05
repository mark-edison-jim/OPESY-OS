#pragma once
#include "ICommand.hpp"
#include "Process.hpp"
#include <string>
#include <utility>

class DeclareCommand : public ICommand {
private:
	std::string text;

	std::string targVar;
	uint16_t exp_value;

	std::atomic<bool> explicitDef = false;
	void declareExplicitVar();
	void assignToVar(uint16_t);
	std::pair<uint16_t, uint16_t> getVariable();

public:
	DeclareCommand(int pid, bool explicitDef, Process* processRef) : ICommand(DECLARE, pid, processRef), explicitDef(explicitDef) {
	};
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