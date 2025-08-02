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
	std::vector<std::shared_ptr<CoreObject>> cores;
	std::vector<std::shared_ptr<std::binary_semaphore>> coreSemaphores;
	std::vector<std::shared_ptr<std::binary_semaphore>> schedSemaphores;
    std::map<std::string, std::shared_ptr<Screen>> screens;
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

        std::ostringstream out;
        std::ostringstream runningPOut;
        std::ostringstream finishedPOut;

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

        for (std::shared_ptr<Process> p : finishedQueue) {
            finishedPOut << std::left << std::setw(12) << p->getName() <<
                std::setw(30) << p->getDate() <<
                std::setw(12) << "Finished" <<
                p->getCommandIndex() << "/" << p->getLinesOfCode() << std::endl;
        }

        out << "CPU Utilization: " << std::fixed << std::setprecision(2) << cputil << " %" << std::endl;
        out << "Cores Used: " + std::to_string(coresUsed) << std::endl;
        out << "Cores Available: " + std::to_string(freeCores) << std::endl << std::endl;
        out << "+-----------------------------------------------------------------------------------------+" << std::endl;
        out << "Running Processes:" << std::endl;
        out << runningPOut.str() << std::endl;
        out << "Finished Processes:" << std::endl;
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
        std::lock_guard<std::mutex> coresLock(coresMtx);
        uint64_t totalCoreCycle = 0;
        for (std::shared_ptr<CoreObject> core : cores) {
            totalCoreCycle+=core->getCoreCycle();
        }
        return totalCoreCycle;
    }

    uint64_t getActiveCoreTicks() {
        std::lock_guard<std::mutex> coresLock(coresMtx);
        uint64_t totalCoreCycle = 0;
        for (std::shared_ptr<CoreObject> core : cores) {
            totalCoreCycle += core->getActiveCoreCycle();
        }
        return totalCoreCycle;
    }
    
    std::vector<uint64_t> getTickInfo() {
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

        out << "+-----------------------------------------------------------------------------------------+" << std::endl << std::endl;
        out << "Total Memory: " << totalMemory << "B" << std::endl;
        out << "Used Memory: " << overallUsedMemory << "B" << std::endl;
        out << "Free Memory: " << freeMemory << "B" << std::endl;
        out << "Idle CPU Ticks: " << std::to_string(tickInfo[0]) << std::endl;
        out << "Active CPU Ticks: " << std::to_string(tickInfo[1]) << std::endl;
        out << "Total CPU Ticks: " << std::to_string(tickInfo[2]) << std::endl;
        out << "Num Paged-in: " << memAcc->getPagedIns() << std::endl;
        out << "Num Paged-out: " << memAcc->getPagedOuts() << std::endl;
        out << "+-----------------------------------------------------------------------------------------+" << std::endl << std::endl;

        return out;

    }

    std::ostringstream getPSMIStats() {
        std::lock_guard<std::mutex> finishedLock(finishedMtx);
        std::lock_guard<std::mutex> coresLock(coresMtx);

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
                memUsage << usedMem * memPerBlock << "B / " << p->getMemorySize() << "B";

                runningPOut << std::left << std::setw(12) << p->getName()
                    << std::setw(20) << memUsage.str() << std::endl;
            }
        }


        int freeCores = totalCores - coresUsed;
        double cputil = (static_cast<double>(coresUsed) / totalCores) * 100.0;
        
        int numUsedMem = memAcc->calculateOverallUsedMemory();
        double usagePercent = (static_cast<double>(numUsedMem * memPerBlock) / (static_cast<double>(totalMemory))) * 100.0;

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
        out << "Memory Usage: " << numUsedMem * memPerBlock << "B / " << totalMemory << "B" << std::endl;
        out << "Memory Util: " << std::fixed << std::setprecision(2) << usagePercent << "%" << std::endl;
        out << "+=========================================================================================+" << std::endl;
        out << "Running Processes and memory usage:" << std::endl;
        out << "+-----------------------------------------------------------------------------------------+" << std::endl;
        out << runningPOut.str() << std::endl;
        out << "+-----------------------------------------------------------------------------------------+" << std::endl << std::endl;

        return out;
    }

};