#include "AddCommand.hpp"
#include "Process.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <random>
#include <iterator>

std::pair<uint16_t, uint16_t> AddCommand::getVariable() {
    // Ensure at least 2 variables exist
    while (processRef->getSymbolTableSize() < 2) {
        std::string varName = "var" + std::to_string(processRef->getVarCount());
        uint16_t val = getRandomUint16();
        processRef->incrementVarCount();
        processRef->loadToPhysMem(varName, val);
    }

    auto symbolTable = processRef->getSymbolTable();
    int tableSize = symbolTable.size();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, tableSize - 1);

    int index1 = distr(gen), index2;
    do {
        index2 = distr(gen);
    } while (index2 == index1);

    auto it1 = symbolTable.begin();
    std::advance(it1, index1);
    auto it2 = symbolTable.begin();
    std::advance(it2, index2);

    uint16_t value1 = processRef->getFromPhysMem(it1->first);
    uint16_t value2 = processRef->getFromPhysMem(it2->first);

    return { value1, value2 };
}

void AddCommand::computeExplicitValues() {
    auto symbolTable = processRef->getSymbolTable();
    uint16_t left = exp_value1;
    uint16_t right = exp_value2;
    if (!exp_var1.empty())
        left = processRef->getFromPhysMem(exp_var1);
    if (!exp_var2.empty())
        right = processRef->getFromPhysMem(exp_var2);

    processRef->loadToPhysMem(targVar, left + right);
    //(*symbolTable)[targVar] = left + right;
}

void AddCommand::assignToVar(uint16_t result){
    int tableSize = processRef->getSymbolTableSize();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, tableSize - 1);

    int index = distr(gen);
    auto symbolTable = processRef->getSymbolTable();
    auto it = symbolTable.begin();
    std::advance(it, index);

    processRef->loadToPhysMem(it->first, result);
    //it->second = result;
}

void AddCommand::execute(int cpuCoreID) {
	if (explicitDef.load()) {
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