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
        // given memAddress
        uint16_t max = processRef->getMemorySize();
        uint16_t valToWrite = processRef->getFromPhysMem(rmaSource);
        processRef->loadToPhysMemAddress(rmaTarget, valToWrite);

    }
    else {
        // given actual int
        uint16_t max = processRef->getMemorySize();
        processRef->loadToPhysMemAddress(rmaTarget, getRandomUint16());
    }
}

void WriteCommand::writeExplicitVal() {
    if (useVar) {
        if (!processRef->getSymbolTable().contains(value) && processRef->checkForSTSpace()) {
            uint16_t val = getRandomUint16();
            processRef->incrementVarCount();
            processRef->loadToPhysMem(value, val);
        }

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