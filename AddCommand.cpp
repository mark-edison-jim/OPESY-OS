#include "AddCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <random>
#include <iterator>

std::pair<uint16_t, uint16_t> AddCommand::getVariable() {

    if (symbolTable->empty()) {
        std::string varName = "var0";
        uint16_t val1 = getRandomUint16();
        symbolTable->insert({ varName, val1 });
    }
	if (symbolTable->size() < 2) {
        std::string varName = "var1";
        uint16_t val2 = getRandomUint16();
        symbolTable->insert({ varName, val2 });
	}

    int tableSize = symbolTable->size();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, tableSize - 1);

    // Select two distinct indices
    int index1 = distr(gen);
    int index2;
    do {
        index2 = distr(gen);
    } while (index2 == index1);

    auto it1 = symbolTable->begin();
    std::advance(it1, index1);

    auto it2 = symbolTable->begin();
    std::advance(it2, index2);

    uint16_t value1 = static_cast<uint16_t>(it1->second);
    uint16_t value2 = static_cast<uint16_t>(it2->second);

    return { value1, value2 };
}

void AddCommand::computeExplicitValues() {
    uint16_t left = exp_value1;
    uint16_t right = exp_value2;
    if (!exp_var1.empty())
        left = (*symbolTable)[exp_var1];
    if (!exp_var2.empty())
        right = (*symbolTable)[exp_var2];
    (*symbolTable)[targVar] = left + right;
}

void AddCommand::assignToVar(uint16_t result){
    int tableSize = symbolTable->size();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, tableSize - 1);

    int index = distr(gen);
    auto it = symbolTable->begin();
    std::advance(it, index);

    it->second = result;
}


AddCommand::AddCommand(int pid, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable, bool explicitDef) : ICommand(ADD, pid, symbolTable), explicitDef(explicitDef) {
	this->text = text;
}

void AddCommand::execute(int cpuCoreID) {
	if (explicitDef) {
		computeExplicitValues();
		return;
	}
	//ICommand::execute();
    std::pair<uint16_t, uint16_t> values = getVariable();
    uint16_t firstValue = values.first;
    uint16_t secondValue = values.second;

    uint32_t result = static_cast<uint32_t>(firstValue) + static_cast<uint32_t>(secondValue);

    if (result > UINT16_MAX) {
        result = UINT16_MAX;
    }
        
	assignToVar(static_cast<uint16_t>(result));
	//logText = "Added " + std::to_string(firstValue) + " and " + std::to_string(secondValue) + ", result: " + std::to_string(result);
}