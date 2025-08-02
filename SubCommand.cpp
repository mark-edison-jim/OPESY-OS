#include "SubCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <random>
#include <iterator>

std::pair<uint16_t, uint16_t> SubCommand::getVariable() {
    // Ensure at least 2 variables exist
    while (processRef->getSymbolTableSize() < 2) {
        std::string varName = "var" + std::to_string(processRef->getVarCount());
        uint16_t val = getRandomUint16();
        processRef->incrementVarCount();
        processRef->loadToPhysMem(varName, val);
    }

    int tableSize = processRef->getSymbolTableSize();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, tableSize - 1);

    int index1 = distr(gen);
    int index2;
    do {
        index2 = distr(gen);
    } while (index2 == index1);

    auto symbolTable = processRef->getSymbolTable();
    auto it1 = symbolTable.begin();
    std::advance(it1, index1);

    auto it2 = symbolTable.begin();
    std::advance(it2, index2);

    uint16_t value1 = processRef->getFromPhysMem(it1->first);
    uint16_t value2 = processRef->getFromPhysMem(it2->first);

    return { value1, value2 };
}


//void SubCommand::assignToVar(uint16_t result) {
//    int tableSize = symbolTable->size();
//
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_int_distribution<> distr(0, tableSize - 1);
//
//    int index = distr(gen);
//    auto it = symbolTable->begin();
//    std::advance(it, index);
//
//    it->second = result;
//}

void SubCommand::assignToVar(uint16_t result) {
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

void SubCommand::computeExplicitValues() {
    auto symbolTable = processRef->getSymbolTable();
    uint16_t left = exp_value1;
    uint16_t right = exp_value2;
    if (!exp_var1.empty())
        left = processRef->getFromPhysMem(exp_var1);
    if (!exp_var2.empty())
        right = processRef->getFromPhysMem(exp_var2);

    processRef->loadToPhysMem(targVar, left - right);
    //(*symbolTable)[targVar] = left + right;

}
//void SubCommand::computeExplicitValues() {
//    uint16_t left = exp_value1;
//    uint16_t right = exp_value2;
//    if (!exp_var1.empty())
//        left = (*symbolTable)[exp_var1];
//    if (!exp_var2.empty())
//        right = (*symbolTable)[exp_var2];
//    (*symbolTable)[targVar] = left - right;
//}

//SubCommand::SubCommand(int pid, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable, bool explicitDef) : ICommand(SLEEP, pid, symbolTable), explicitDef(explicitDef) {
//    this->text = text;
//}

void SubCommand::execute(int cpuCoreID) {
    if (explicitDef.load()) {
        computeExplicitValues();
        return;
    }
    //ICommand::execute();
    std::pair<uint16_t, uint16_t> values = getVariable();
    uint16_t firstValue = values.first;
    uint16_t secondValue = values.second;

    uint16_t higher = (((firstValue) > (secondValue)) ? (firstValue) : (secondValue));
    uint16_t lower = (((firstValue) < (secondValue)) ? (firstValue) : (secondValue));

    uint16_t result = higher - lower;

    assignToVar(result);
}