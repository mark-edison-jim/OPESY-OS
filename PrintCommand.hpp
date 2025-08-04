#pragma once
#include "ICommand.hpp"
#include "Process.hpp"
#include <string>
#include <utility>

class PrintCommand : public ICommand {
private:
	std::string text;
	std::pair<uint16_t, uint16_t> getVariable();
	std::string targVar;
	std::atomic<bool> explicitDef = false;

public:
	PrintCommand(int pid, const std::string& text, bool explicitDef, Process* processRef) : ICommand(PRINT, pid, processRef), explicitDef(explicitDef) {
		this->text = text;
	};
	void execute(int) override;
	std::string getText() override{
		return text;
	}
	std::string getLog() override {
		return logText;
	}
	void setExplicit(std::string target, std::string text) {
		explicitDef = true;
		this->text = text;
		targVar = target;
	}
};