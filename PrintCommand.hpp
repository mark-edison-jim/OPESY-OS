#pragma once
#include "ICommand.hpp"
#include <string>

class PrintCommand : public ICommand {
private:
	std::string text;

public:
	PrintCommand(int pid, std::string& text, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable);
	void execute(int) override;
	std::string getText() override{

		return text;
	}
};