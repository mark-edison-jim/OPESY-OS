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
	std::atomic<bool> processAbrupted = false;
	std::atomic<uint64_t> coreCycle = 0;
	std::atomic<uint64_t> activeCoreCycle = 0;
    std::shared_ptr<Process> process;
    std::shared_ptr<std::binary_semaphore> coreSemaphore{0};
    std::shared_ptr<std::binary_semaphore> schedSemaphore{0};
    std::atomic<int> quantumCycleCounter = 0;
    std::mutex processMutex;
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
        return quantumCycleCounter.load();
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

    std::atomic<uint64_t> getCoreCycle() const {
		return coreCycle.load();
    }

    std::atomic<uint64_t> getActiveCoreCycle() const {
        return activeCoreCycle.load();
    }

    void initializeProcess();
    
    void resetCoreCycle() {
        coreCycle = 0;
    }

	int getCoreID() const {
		return coreID;
	}

    bool isIdle() const {
		return processFinished.load();
    }

    void flushProcessMemory() {
        process->deallocateMemory();
    }
    
    bool getProcessAbrupt() {
        return processAbrupted.load();
    }

    std::shared_ptr<Process> getProcess() {
        std::lock_guard<std::mutex> lock(processMutex);
        return process;
    }
};
