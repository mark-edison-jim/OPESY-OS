#include "WriteCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <random>
#include <iterator>

void WriteCommand::writeVal() {
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
        writeVal();
    }
    else {
        std::cerr << "[ERROR] No address/value specified for WRITE." << std::endl;
    }
}