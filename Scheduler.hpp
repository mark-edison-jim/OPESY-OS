#pragma once

#include <stdexcept>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <memory>
#include "Process.hpp"
#include "CoreObject.hpp"
#include "screenTerminal.hpp"
#include <map>
#include <sstream>
#include <iomanip>
#include "MemoryAllocator.hpp"


class Scheduler : public std::enable_shared_from_this<Scheduler> {
private:
    uint64_t cpuCycle = 0;
    std::atomic<bool> makeProcesses = false;
    std::queue<std::shared_ptr<Process>> processQueue;
    std::vector<std::shared_ptr<Process>> finishedQueue;
    std::vector<std::shared_ptr<Process>> waitingQueue;
	std::vector<std::shared_ptr<CoreObject>> cores;
	std::vector<std::shared_ptr<std::binary_semaphore>> coreSemaphores;
	std::vector<std::shared_ptr<std::binary_semaphore>> schedSemaphores;
    std::map<std::string, std::shared_ptr<Screen>> screens;
    std::mutex waitingMtx;
    std::mutex processMtx;
    std::mutex coresMtx;
    std::mutex finishedMtx;
    std::mutex screensMtx;
    std::condition_variable cv;
	uint64_t minInstructions;
    uint64_t maxInstructions;
    uint32_t batchFreq;
    int currProcIdx = 0;
    //int numCommands;
    int totalCores;
    size_t latestProcessID = 0;
    std::atomic<int> freeCores;
    int numProcesses;
    int execDelay;
    std::string activeScreen;
    std::string mode;
    int quantum_cycle;
	bool exitOS = false;
    uint16_t totalMemory;
    uint16_t memPerBlock;
    uint16_t maxMemPerBlock;
    uint16_t minMemPerProcess;
    std::shared_ptr<MemoryAllocator> memAcc;
    //std::vector<bool> coreBusy;

public:    
    Scheduler(int availableCores, int numProcesses, uint64_t minIns, uint64_t maxIns, int execDelay, int batchFreq, std::string mode, int quantum, uint16_t totalMemory, uint16_t memPerBlock, uint16_t minMemPerProcess, uint16_t maxMemPerBlock)
        : minInstructions(minIns), maxInstructions(maxIns), totalCores(availableCores), freeCores(availableCores), numProcesses(numProcesses), execDelay(execDelay), batchFreq(batchFreq), 
        mode(mode), quantum_cycle(quantum), totalMemory(totalMemory), memPerBlock(memPerBlock), memAcc(std::make_shared<MemoryAllocator>(totalMemory, memPerBlock)), minMemPerProcess(minMemPerProcess), maxMemPerBlock(maxMemPerBlock){
        if (availableCores <= 0) {
            throw std::invalid_argument("Number of cores must be greater than zero.");
        }   

        for (int i = 0; i < totalCores; ++i) {
			coreSemaphores.push_back(std::make_shared<std::binary_semaphore>(0));
            schedSemaphores.push_back(std::make_shared<std::binary_semaphore>(0));
			cores.push_back(std::make_unique<CoreObject>(i, execDelay, coreSemaphores[i], schedSemaphores[i]));
        }
    }
    const std::queue<std::shared_ptr<Process>>& getProcessQueue() const {
        return processQueue;
    }
    
	void stopOS() {
		exitOS = true;
	}

    const std::vector<std::shared_ptr<Process>>& getFinishedQueue() const {
        return finishedQueue;
    }

	void deleteScreen(const std::string& name) {
		std::lock_guard<std::mutex> screensLock(screensMtx);
		auto it = screens.find(name);
		if (it != screens.end()) {
			screens.erase(it);
		}
	}

    const int getFreeCores() const {
        return freeCores;
    }

    const std::vector<std::shared_ptr<CoreObject>> getCoresArray() {
		std::lock_guard<std::mutex> coresLock(coresMtx);
        return cores;
    }

    void assignNewProcesses();
    void fcfs();
    void checkRoundRobin();
    void checkCoreFinished();
    //bool getNextProcess(std::shared_ptr<Process>& out);
    void addProcess(std::string, uint16_t);
    void addProcess(std::string, uint16_t, std::string);
    void generateProcess();

    void makeProcess() {
		if (!makeProcesses)
            makeProcesses = true;
	}

    void stopMakingProcess() {
        if (makeProcesses)
            makeProcesses = false;
    }

    std::string getActiveScreen() const {
        return activeScreen;
    }
    
    std::shared_ptr<Screen> getScreen() {
        std::lock_guard<std::mutex> screensLock(screensMtx);
        auto it = screens.find(activeScreen);
        if (it != screens.end()) {
            return it->second;
        }
        else {
            return nullptr;
        }
    }

    std::shared_ptr<Screen> getScreen(const std::string& name) {
        std::lock_guard<std::mutex> screensLock(screensMtx);
        auto it = screens.find(name);
        if (it != screens.end()) {
            return it->second;
        }
        else {
            return nullptr;
        }
    }

    std::ostringstream getProcessStats() {
        std::lock_guard<std::mutex> finishedLock(finishedMtx);
        std::lock_guard<std::mutex> coresLock(coresMtx);
        std::lock_guard<std::mutex> waitingLock(waitingMtx);

        std::ostringstream out;
        std::ostringstream runningPOut;
        std::ostringstream finishedPOut;
        std::ostringstream waitingPOut;

        int coresUsed = 0;
        for (std::shared_ptr<CoreObject> core : cores) {
            std::shared_ptr<Process> p = core->getProcess();
            if (p) {
                coresUsed++;
                runningPOut << std::left << std::setw(12) << p->getName() <<
                    std::setw(30) << p->getDate() <<
                    std::setw(12) << "Core: " + std::to_string(p->getCpuCoreID()) <<
                    p->getCommandIndex() << "/" << p->getLinesOfCode() << std::endl;
            }
        }

        int freeCores = totalCores - coresUsed;
        double cputil = (static_cast<double>(coresUsed) / totalCores) * 100.0;

        int f = 0;
        for (std::shared_ptr<Process> p : finishedQueue) {
            if (f < 10) {
                if (p->getState() == 4) {
                    finishedPOut << std::left << std::setw(12) << p->getName() <<
                        std::setw(30) << p->getDate() <<
                        std::setw(12) << "Abrupted" <<
                        p->getCommandIndex() << "/" << p->getLinesOfCode() << std::endl;
                    f++;
                }
                else {
                    finishedPOut << std::left << std::setw(12) << p->getName() <<
                        std::setw(30) << p->getDate() <<
                        std::setw(12) << "Finished" <<
                        p->getCommandIndex() << "/" << p->getLinesOfCode() << std::endl;
                    f++;
                }
            }
            else
                break;
        }

        int i = 0;
        for (std::shared_ptr<Process> p : waitingQueue) {
            if (i < 10) {
                if (p) {
                    waitingPOut << std::left << std::setw(12) << p->getName() <<
                        std::setw(30) << p->getDate() <<
                        p->getCommandIndex() << "/" << p->getLinesOfCode() << std::endl;
                    i++;
                }
            }
            else
                break;
        }

        out << "CPU Tick: " + std::to_string(cpuCycle) << std::endl;
        out << "Core Tick: " + std::to_string(cores[0]->getCoreCycle()) << std::endl;
        out << "CPU Utilization: " << std::fixed << std::setprecision(2) << cputil << " %" << std::endl;
        out << "Cores Used: " + std::to_string(coresUsed) << std::endl;
        out << "Cores Available: " + std::to_string(freeCores) << std::endl << std::endl;
        out << "+-----------------------------------------------------------------------------------------+" << std::endl;
        out << "Running Processes:" << std::endl;
        out << runningPOut.str() << std::endl;
        out << "Waiting Processes (Showing max(" << std::to_string(i) << ") of " << std::to_string(waitingQueue.size()) << "):" << std::endl;
        out << waitingPOut.str() << std::endl;
        out << "Finished Processes (Showing max(" << std::to_string(f) << ") of " << std::to_string(finishedQueue.size()) << "):" << std::endl;
        out << finishedPOut.str() << std::endl;
        out << "+-----------------------------------------------------------------------------------------+" << std::endl << std::endl;

        return out;
    }

    int getUsedMemForPID(int pid) {
        return memAcc->calculatePIDUsedMemory(pid);
    }

    void setActiveScreen(std::string screenName) {
        activeScreen = screenName;
    }

    bool findScreen(std::string name) {
        std::lock_guard<std::mutex> screensLock(screensMtx);
        return screens.find(name) != screens.end();
    }

    std::string getInvalidScreenMem(std::string name) {
        std::lock_guard<std::mutex> screensLock(screensMtx);
        std::stringstream invalidMessage;
        invalidMessage<< "Process <screen." << name << "> shut down due to memory access violation error that occurred at <" << screens[name]->getTimeInvalid() << ">. <" 
            << screens[name]->getInvalidAddress() << "> invalid.";
        return invalidMessage.str();
    }

    std::shared_ptr<Screen> addScreen(const std::string& name, int pid, uint64_t totalLines) {
        std::lock_guard<std::mutex> screensLock(screensMtx);
        auto screen = std::make_shared<Screen>(name, pid, totalLines);
        screens[name] = screen;
        return screen;
    }

    void addCommandToScreen(const std::string& command, const std::string& output) {
        std::lock_guard<std::mutex> screensLock(screensMtx);
        screens[activeScreen]->addCommand(command, output);
    }

	void clearCommandHistory() {
		std::lock_guard<std::mutex> screensLock(screensMtx);
		screens[activeScreen]->clearHistory();
	}

    uint64_t getTotalCoreTicks() {
        uint64_t totalCoreCycle = 0;
        for (std::shared_ptr<CoreObject> core : cores) {
            totalCoreCycle+=core->getCoreCycle();
        }
        return totalCoreCycle;
    }

    uint64_t getActiveCoreTicks() {
        uint64_t totalCoreCycle = 0;
        for (std::shared_ptr<CoreObject> core : cores) {
            totalCoreCycle += core->getActiveCoreCycle();
        }
        return totalCoreCycle;
    }
    
    std::vector<uint64_t> getTickInfo() {
        if(!memAcc->getConfig())
            std::lock_guard<std::mutex> coresLock(coresMtx);
        uint64_t totalCoreCycles = getTotalCoreTicks();
        uint64_t activeCoreCycles = getActiveCoreTicks();
        uint64_t idleCoreCycles = totalCoreCycles - activeCoreCycles;

        std::vector<uint64_t> coreInfo = { idleCoreCycles, activeCoreCycles, totalCoreCycles };
        return coreInfo;
    }

    std::ostringstream getVmStats() {
        std::ostringstream out;
        std::vector<uint64_t> tickInfo = getTickInfo();

        int overallUsedFrames = memAcc->calculateOverallUsedMemory();
        int overallUsedMemory = overallUsedFrames * memPerBlock;
        int freeMemory = totalMemory - overallUsedMemory;
        const int labelWidth = 20;
        out << "+-----------------------------------------------------------------------------------------+" << std::endl;

        out << std::right << std::setw(labelWidth) << "Total Memory:" << " " << totalMemory << " B" << std::endl;
        out << std::right << std::setw(labelWidth) << "Used Memory:" << " " << overallUsedMemory << " B" << std::endl;
        out << std::right << std::setw(labelWidth) << "Free Memory:" << " " << freeMemory << " B" << std::endl;

        out << std::right << std::setw(labelWidth) << "Idle CPU Ticks:" << " " << tickInfo[0] << std::endl;
        out << std::right << std::setw(labelWidth) << "Active CPU Ticks:" << " " << tickInfo[1] << std::endl;
        out << std::right << std::setw(labelWidth) << "Total CPU Ticks:" << " " << tickInfo[2] << std::endl;

        out << std::right << std::setw(labelWidth) << "Num Paged-in:" << " " << memAcc->getPagedIns() << std::endl;
        out << std::right << std::setw(labelWidth) << "Num Paged-out:" << " " << memAcc->getPagedOuts() << std::endl;

        out << "+-----------------------------------------------------------------------------------------+" << std::endl << std::endl;

        return out;
        // screen -c faulty_process 256 "DECLARE varA 10; DECLARE varB 5; ADD varA varA varB; WRITE 0x0500 varA; READ varC 0x0500; PRINT (\"Variable A: \"+varA); PRINT (\"Variable C: \"+varC)"
        // screen -c faulty_process2 256 "DECLARE varA 10; DECLARE varB 5; ADD varA varA varB; WRITE 0x0050 varA; READ varC 0x0050; PRINT (\"Variable A: \"+varA); PRINT (\"Variable C: \"+varC)"
    }

    std::ostringstream getPSMIStats() {
        if (!memAcc->getConfig()) {
            std::lock_guard<std::mutex> finishedLock(finishedMtx);
            std::lock_guard<std::mutex> coresLock(coresMtx);
        }
        std::ostringstream out;
        std::ostringstream runningPOut;
        //std::ostringstream finishedPOut;

        int coresUsed = 0;
        for (std::shared_ptr<CoreObject> core : cores) {
            std::shared_ptr<Process> p = core->getProcess();
            if (p) {
                int usedMem = getUsedMemForPID(p->getPID());
                coresUsed++;

                std::ostringstream memUsage;
                memUsage << usedMem * memPerBlock << " B / " << p->getMemorySize() << " B";

                runningPOut << std::left << std::setw(12) << p->getName()
                    << std::setw(20) << memUsage.str() << std::endl;
            }
        }

        double cputil = (static_cast<double>(coresUsed) / totalCores) * 100.0;
        
        double usagePercent = (static_cast<double>(coresUsed * memPerBlock) / (static_cast<double>(totalMemory))) * 100.0;

        //for (std::shared_ptr<Process> p : finishedQueue) {
        //    finishedPOut << std::left << std::setw(12) << p->getName() <<
        //        std::setw(30) << p->getDate() <<
        //        std::setw(12) << "Finished" <<
        //        p->getCommandIndex() << "/" << p->getLinesOfCode() << std::endl;
        //}

        out << "+-----------------------------------------------------------------------------------------+" << std::endl;
        out << "|                        PROCESS-SMI v01.00 Driver Version: 01.00                         |" << std::endl;
        out << "+-----------------------------------------------------------------------------------------+" << std::endl;
        out << "CPU Utilization: " << std::fixed << std::setprecision(2) << cputil << " %" << std::endl;
        out << "Memory Usage: " << coresUsed * memPerBlock << " B / " << totalMemory << " B" << std::endl;
        out << "Memory Util: " << std::fixed << std::setprecision(2) << usagePercent << "%" << std::endl;
        out << "+=========================================================================================+" << std::endl;
        out << "Running Processes and memory usage:" << std::endl;
        out << "+-----------------------------------------------------------------------------------------+" << std::endl;
        out << runningPOut.str() << std::endl;
        out << "+-----------------------------------------------------------------------------------------+" << std::endl << std::endl;

        return out;
    }

};