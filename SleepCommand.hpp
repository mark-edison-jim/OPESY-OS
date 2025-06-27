#pragma once
#include "ICommand.hpp"
#include <string>
#include <utility>

class SleepCommand : public ICommand {
private:
	std::string text;

	uint8_t sleepTime;
	uint8_t exp_sleepTime;

	void assignToVar(uint16_t);
	std::pair<uint16_t, uint16_t> getVariable();
	bool explicitDef = false;
public:
	SleepCommand(int pid, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable, bool explicitDef);
	void execute(int) override;
	std::string getText() override {
		return text;
	}
	std::string getLog() override {
		return logText;
	}
	uint8_t getSleepTime() const {
		if (explicitDef)
			return exp_sleepTime;
		else
			return sleepTime;
	}
	void setExplicit(uint8_t sleepTime) {
		explicitDef = true;
		exp_sleepTime = sleepTime;
	}
};