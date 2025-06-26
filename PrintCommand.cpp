#include "PrintCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <random>
#include <iterator>

std::pair<uint16_t, uint16_t> PrintCommand::getVariable(){

	int tableSize = symbolTable->size();

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distr(0, tableSize - 1);

	int index = distr(gen);
	auto it = symbolTable->begin();
	std::advance(it, index);

	std::hash<std::string> hasher;
	uint16_t hashedKey = hasher(it->first);

	uint16_t value = static_cast<uint16_t>(it->second);

	return { value, 0 };
}

PrintCommand::PrintCommand(int pid, std::string& text, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable) : ICommand(PRINT, pid, symbolTable) {
	this->text = text;
}

void PrintCommand::execute(int cpuCoreID) {
	//ICommand::execute();

	logText = "";
	if (fiftyFiftyChance()) {
		if (!symbolTable->empty()) {
			uint16_t value = getVariable().first;
			std::string msg = "Value from : " + std::to_string(value);
			text = msg;
		}
	}
	logText = text;
}