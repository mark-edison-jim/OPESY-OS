#pragma once
#include <string>
#include <atomic>
#include "ICommand.hpp"
#include "Process.hpp"
using namespace std;

class ReadCommand : public ICommand {
private:
    std::string targVar;
    string memAddress; 
    std::atomic<bool> explicitDef = false;

    void readVar();
    void readExplicitVar();
    std::pair<uint16_t, uint16_t> getVariable();

public:
    ReadCommand(int pid, bool explicitDef, Process* processRef)
        : ICommand(READ, pid, processRef), explicitDef(explicitDef){
        uint16_t max = processRef->getMemorySize();
        memAddress = intToHex(getRandomEvenFrom16Range(0, max - 2), 4);
    
    }

    void setExplicit(const std::string& target, string address) {
        explicitDef = true;
        targVar = target;
        memAddress = address;
    }

    bool isInvalidMem() {
        return static_cast<uint16_t>(std::stoul(memAddress, nullptr, 16)) >= processRef->getMemorySize();
    }

    void execute(int cpuCoreID) override;

    std::string getText() override { return "READ " + targVar + " 0x" + memAddress; }
    std::string getLog() override { return logText; }
};