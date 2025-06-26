#include "SubCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <random>
#include <iterator>

std::pair<uint16_t, uint16_t> SubCommand::getVariable() {

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

void SubCommand::assignToVar(uint16_t result) {
    int tableSize = symbolTable->size();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, tableSize - 1);

    int index = distr(gen);
    auto it = symbolTable->begin();
    std::advance(it, index);

    it->second = result;
}

SubCommand::SubCommand(int pid, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable) : ICommand(SUBTRACT, pid, symbolTable) {
    this->text = text;
}

void SubCommand::execute(int cpuCoreID) {
    //ICommand::execute();
    std::pair<uint16_t, uint16_t> values = getVariable();
    uint16_t firstValue = values.first;
    uint16_t secondValue = values.second;

	uint16_t higher = std::max(firstValue, secondValue);
	uint16_t lower = std::min(firstValue, secondValue);

    uint16_t result = higher - lower;

    assignToVar(result);
}