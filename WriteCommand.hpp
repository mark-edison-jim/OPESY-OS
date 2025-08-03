#pragma once
#include <string>
#include <atomic>
#include "ICommand.hpp"
#include "Process.hpp"

class WriteCommand : public ICommand {
private:
    std::string memAddress;   // hex string e.g., "0x1000"
    std::string value;        // int or string
    bool useVar = false;      // true if writing from variable
    std::atomic<bool> explicitDef = false;

    void writeVal();
    std::pair<uint16_t, uint16_t> getVariable();

public:
    WriteCommand(int pid, bool explicitDef, Process* processRef)
        : ICommand(WRITE, pid, processRef), explicitDef(explicitDef) { }

    void setExplicit(std::string address, std::string val) {
        explicitDef = true;
        memAddress = address;
        value = val;
        if (std::isdigit(value[0])) { // starts with int or not
            useVar = true;
        }
        else {
            useVar = false;
        }
    }

    void execute(int cpuCoreID) override;

    std::string getText() override { return "WRITE " + memAddress; }
    std::string getLog() override { return logText; }
};