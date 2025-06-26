#include "DeclareCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <random>
#include <iterator>

void DeclareCommand::assignToVar(uint16_t result) {
    std::string varName = "var0";
    uint16_t val1 = getRandomUint16();
    symbolTable->insert({ varName, result });
}

std::pair<uint16_t, uint16_t> DeclareCommand::getVariable()
{
    return std::pair<uint16_t, uint16_t>();
}


DeclareCommand::DeclareCommand(int pid, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable) : ICommand(DECLARE, pid, symbolTable) {
}

void DeclareCommand::execute(int cpuCoreID) {
    //ICommand::execute(); 
    if(symbolTable->size() < 32)
        assignToVar(getRandomUint16());
}