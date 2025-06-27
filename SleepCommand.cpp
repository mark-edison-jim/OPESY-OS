#include "SleepCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <random>
#include <iterator>

void SleepCommand::assignToVar(uint16_t result) {

}

std::pair<uint16_t, uint16_t> SleepCommand::getVariable()
{
    return std::pair<uint16_t, uint16_t>();
}


SleepCommand::SleepCommand(int pid, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable, bool explicitDef) : ICommand(SLEEP, pid, symbolTable), explicitDef(explicitDef), sleepTime(getRandomUint8()) {

}

void SleepCommand::execute(int cpuCoreID) {
    //ICommand::execute(); 
    if (explicitDef)
        exp_sleepTime -= 1;
    else
        sleepTime -= 1;
}