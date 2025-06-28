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

class Scheduler : public std::enable_shared_from_this<Scheduler> {
private:
    int cpuCycle = 0;
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
    int latestProcessID = 0;
    std::atomic<int> freeCores;
    int numProcesses;
    int execDelay;
    std::string activeScreen;
    std::string mode;
    int quantum_cycle;
	bool exitOS = false;
    //std::vector<bool> coreBusy;

public:    
    Scheduler(int availableCores, int numProcesses, uint64_t minIns, uint64_t maxIns, int execDelay, int batchFreq, std::string mode, int quantum)
        : minInstructions(minIns), maxInstructions(maxIns), totalCores(availableCores), freeCores(availableCores), numProcesses(numProcesses), execDelay(execDelay), batchFreq(batchFreq), mode(mode), quantum_cycle(quantum){
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
    void addProcess(std::string processName);
    void generateFixedProcesses();
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

};