#pragma once
#include "Process.hpp"
#include <string>
#include <atomic>
#include "ICommand.hpp"

class WriteCommand : public ICommand {
private:
    std::string memAddress;   // hex string e.g., "0x1000"
    std::string rmaTarget;   // target hex string e.g., "0x1000"
    std::string value;        // int or string
    bool useVar = false;      // true if writing from variable
    std::atomic<bool> explicitDef = false;

    void writeVal();
    void writeExplicitVal();
    std::pair<uint16_t, uint16_t> getVariable();

public:
    WriteCommand(int pid, bool explicitDef, Process* processRef)
        : ICommand(WRITE, pid, processRef), explicitDef(explicitDef) {

        uint16_t max = processRef->getMemorySize();

        rmaTarget = intToHex(getRandomEvenFrom16Range(0, max - 2), 4);

    }

    void setExplicit(std::string address, std::string val) {
        explicitDef = true;
        memAddress = address;
        value = val;
        useVar = !std::isdigit(value[0]); // starts with int or not
    }

    bool isInvalidMem() {
        if (explicitDef)
            return static_cast<uint16_t>(std::stoul(memAddress, nullptr, 16)) >= processRef->getMemorySize();
        else
            return static_cast<uint16_t>(std::stoul(rmaTarget, nullptr, 16)) >= processRef->getMemorySize();
    }

    std::string getMemAddress() {
        if (explicitDef)
            return memAddress;
        else
            return rmaTarget;
    }

    void execute(int cpuCoreID) override;

    std::string getText() override { return "WRITE " + memAddress; }
    std::string getLog() override { return logText; }
};