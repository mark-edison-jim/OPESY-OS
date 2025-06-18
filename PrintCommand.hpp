#pragma once
#include "ICommand.hpp"
#include <string>

class PrintCommand : public ICommand {
private:
	std::string text;

public:
	PrintCommand(int pid, std::string& text);
	void execute() override;
	std::string getText() override{
		return text;
	}
};