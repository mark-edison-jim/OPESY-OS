#include "DeclareCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <random>
#include <iterator>

void DeclareCommand::declareExplicitVar(){
    processRef->loadToPhysMem(targVar, exp_value);
    //(*symbolTable)[targVar] = exp_value;
}

void DeclareCommand::assignToVar(uint16_t result) {
    std::string varName = "var" + std::to_string(processRef->getVarCount());
    uint16_t val1 = getRandomUint16();
    processRef->incrementVarCount();
    processRef->loadToPhysMem(varName, result);
    //symbolTable->insert({ varName, result });
}

std::pair<uint16_t, uint16_t> DeclareCommand::getVariable()
{
    return std::pair<uint16_t, uint16_t>();
}

void DeclareCommand::execute(int cpuCoreID) {
    //ICommand::execute(); 
    if (processRef->checkForSTSpace()) {
        if (explicitDef.load()) {
			declareExplicitVar();
		}else
            assignToVar(getRandomUint16());
    }
}