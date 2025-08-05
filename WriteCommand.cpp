#include "WriteCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <random>
#include <iterator>

void WriteCommand::writeVal() {
    if (fiftyFiftyChance()) {
        // given var

        int tableSize = processRef->getSymbolTableSize();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(0, tableSize - 1);

        int index = distr(gen);
        auto symbolTable = processRef->getSymbolTable();
        auto it = symbolTable.begin();
        std::advance(it, index);

        if (!processRef->getSymbolTable().contains(value) && processRef->checkForSTSpace()) {
            processRef->incrementVarCount();
            processRef->loadToPhysMem(value, 0);
        }

        uint16_t valToWrite = processRef->getFromPhysMem(it->first);
        processRef->loadToPhysMemAddress(rmaTarget, valToWrite);
    }
    else {
        // given actual int
        processRef->loadToPhysMemAddress(rmaTarget, getRandomUint16());
    }
}

void WriteCommand::writeExplicitVal() {
    if (useVar) {
        uint16_t valToWrite = processRef->getFromPhysMem(value);
        processRef->loadToPhysMemAddress(memAddress, valToWrite);
    }
    else {
        // Convert value to uint16_t
        uint16_t valToWrite = static_cast<uint16_t>(std::stoi(value, nullptr, 0));
        processRef->loadToPhysMemAddress(memAddress, valToWrite);
    }
}

std::pair<uint16_t, uint16_t> WriteCommand::getVariable()
{
    return std::pair<uint16_t, uint16_t>();
}

void WriteCommand::execute(int cpuCoreID) {
    if (explicitDef.load()) {
        writeExplicitVal();
    }
    else {
        writeVal();
        //std::cerr << "[ERROR] No address/value specified for WRITE." << std::endl;
    }
}