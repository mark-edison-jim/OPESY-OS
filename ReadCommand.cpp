#include "ReadCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <random>
#include <iterator>

void ReadCommand::readVar() {
    
    if (!processRef->getSymbolTable().contains(targVar) && processRef->checkForSTSpace()) {
        processRef->incrementVarCount();
        processRef->loadToPhysMem(targVar, 0);
    }

    int tableSize = processRef->getSymbolTableSize();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, tableSize - 1);

    int index = distr(gen);
    auto symbolTable = processRef->getSymbolTable();
    auto it = symbolTable.begin();
    std::advance(it, index);

    uint16_t value = 0;

    value = processRef->readFromPhysMem(memAddress);
    processRef->loadToPhysMem(it->first, value);
}

void ReadCommand::readExplicitVar() {

    uint16_t value = 0;

    value = processRef->readFromPhysMem(memAddress);
    processRef->loadToPhysMem(targVar, value);
}

std::pair<uint16_t, uint16_t> ReadCommand::getVariable()
{
    return std::pair<uint16_t, uint16_t>();
}

void ReadCommand::execute(int cpuCoreID) {
    if (explicitDef.load()) {
        readExplicitVar();
    }
    else {
        readVar();
        //std::cerr << "[ERROR] No variable specified for READ." << std::endl;
    }
}