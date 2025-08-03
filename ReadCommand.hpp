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
    std::pair<uint16_t, uint16_t> getVariable();

public:
    ReadCommand(int pid, bool explicitDef, Process* processRef)
        : ICommand(READ, pid, processRef), explicitDef(explicitDef) { }

    void setExplicit(const std::string& target, string address) {
        explicitDef = true;
        targVar = target;
        memAddress = address;
    }

    void execute(int cpuCoreID) override;

    std::string getText() override { return "READ " + targVar + " 0x" + memAddress; }
    std::string getLog() override { return logText; }
};