#include "PrintCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <random>
#include <iterator>

std::pair<uint16_t, uint16_t> PrintCommand::getVariable(){
	int tableSize = processRef->getSymbolTableSize();

	if (tableSize == 0) {
		return { 0, 0 };  // or handle it differently depending on your app's logic
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distr(0, tableSize - 1);

	int index = distr(gen);
	auto symbolTable = processRef->getSymbolTable();
	auto it = symbolTable.begin();
	std::advance(it, index);

	uint16_t value = processRef->getFromPhysMem(it->first);
	return { value, 0 };
}

void PrintCommand::execute(int cpuCoreID) {
	//ICommand::execute();

	logText = "";
	if (explicitDef.load()) {
		uint16_t value = processRef->getFromPhysMem(targVar);
		if (text.empty()) {
			text = std::to_string(value);
		}
		else if (!text.empty() && !targVar.empty()) {
			text += std::to_string(value);
		}
	}else if (fiftyFiftyChance()) {
		if (processRef->getSymbolTableSize() > 0) {
			uint16_t value = getVariable().first;
			std::string msg = "Value from : " + std::to_string(value);
			text = msg;
		}
	}
	logText = text;
}