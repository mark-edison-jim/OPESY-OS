#pragma once

#include "Process.hpp"
#include <memory>
#include <semaphore>
#include <thread>

class CoreObject {
private:
    int coreID;
    int delay;
	std::atomic<bool> processFinished = true;
	std::atomic<int> coreCycle = 1;
    std::shared_ptr<Process> process;
    std::shared_ptr<std::binary_semaphore> coreSemaphore{0};
    std::shared_ptr<std::binary_semaphore> schedSemaphore{0};
    int quantumCycleCounter = 0;

    //std::shared_ptr<Scheduler> scheduler;

public:
    CoreObject(int coreID, int delay, std::shared_ptr<std::binary_semaphore> coreSemaphore, std::shared_ptr<std::binary_semaphore> schedSemaphore) : 
        coreID(coreID), delay(delay), coreSemaphore(coreSemaphore), schedSemaphore(schedSemaphore){
		std::thread processThread([this]() {
			run();
			});
		processThread.detach();
    };
    
    void resetQuantumCounter() {
        quantumCycleCounter = 0;
    }

    int getQuantumCycleCounter() {
        return quantumCycleCounter;
    }

    void incrementQuantumCycleCounter() {
        quantumCycleCounter++;
    }


    void setProcessWait() {
        processFinished = true;
        process->setWaiting();
    }

    void run();

    void assignProcess(std::shared_ptr<Process> p) {
		process = p;
    };

    std::atomic<int> getCoreCycle() const {
		return coreCycle.load();
    }

    void initializeProcess();
    
    void resetCoreCycle() {
        coreCycle = 1;
    }

	int getCoreID() const {
		return coreID;
	}

    bool isIdle() const {
		return processFinished.load();
    }

    std::shared_ptr<Process> getProcess() {
		return process;
    };
};
