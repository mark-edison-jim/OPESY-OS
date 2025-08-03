#include "ReadCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <random>
#include <iterator>
#include <iostream>

void ReadCommand::readVar() {
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
        readVar();
    }
    else {
        std::cerr << "[ERROR] No variable specified for READ." << std::endl;
    }
}