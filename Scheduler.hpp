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

class Scheduler : public std::enable_shared_from_this<Scheduler> {
private:
    int cpuCycle = 1;
    std::atomic<bool> makeProcesses = false;
    std::queue<std::shared_ptr<Process>> processQueue;
    std::vector<std::shared_ptr<Process>> finishedQueue;
	std::vector<std::shared_ptr<CoreObject>> cores;
	std::vector<std::shared_ptr<std::binary_semaphore>> coreSemaphores;
	std::vector<std::shared_ptr<std::binary_semaphore>> schedSemaphores;
    std::map<std::string, std::shared_ptr<Screen>> screens;
    std::mutex mtx;
    std::mutex finishedMtx;
    std::mutex screensMtx;
    std::condition_variable cv;
    uint32_t batchFreq;
    int currProcIdx = 0;
    int numCommands = 100;
    int totalCores;
    int latestProcessID = 0;
    std::atomic<int> freeCores;
    int numProcesses;
    int execDelay;
    std::string activeScreen;
    std::string mode;
    int quantum_cycle;
    //std::vector<bool> coreBusy;

public:
    Scheduler(int availableCores, int numProcesses, int execDelay, int batchFreq, std::string mode, int quantum)
        : totalCores(availableCores), freeCores(availableCores), numProcesses(numProcesses), execDelay(execDelay), batchFreq(batchFreq), mode(mode), quantum_cycle(quantum){
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

    const std::vector<std::shared_ptr<CoreObject>> getCoresArray() const {
        return cores;
    }

    void assignNewProcesses();
    void fcfs();
    void checkRoundRobin();
    void checkCoreFinished();
    //bool getNextProcess(std::shared_ptr<Process>& out);
    void addProcess(std::string processName);
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


    void setActiveScreen(std::string screenName) {
        activeScreen = screenName;
    }

    bool findScreen(std::string name) {
        std::lock_guard<std::mutex> screensLock(screensMtx);
        return screens.find(name) != screens.end();
    }

    std::shared_ptr<Screen> addScreen(const std::string& name, int pid, int totalLines) {
        std::lock_guard<std::mutex> screensLock(screensMtx);
        auto screen = std::make_shared<Screen>(name, pid, totalLines);
        screens[name] = screen;
        return screen;
    }

    void addCommandToScreen(const std::string& command, const std::string& output) {
        std::lock_guard<std::mutex> screensLock(screensMtx);
        screens[activeScreen]->addCommand(command, output);
    }

};