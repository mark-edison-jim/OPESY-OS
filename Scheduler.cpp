#include <stdexcept>
#include "CoreObject.hpp"
#include <thread>
#include <queue>
#include "Process.hpp"
#include <mutex>
#include "Scheduler.hpp"
#include <memory>
#include <iostream>

void Scheduler::fcfs() {

    while (!exitOS) {
        //std::unique_lock<std::mutex> lock(mtx);
        //cv.wait(lock, [&]() {   
        //    return freeCores > 0 || !processQueue.empty();
        //    });

        //checkCoreFinished();

        for (int i = 0; i < totalCores; ++i)
            coreSemaphores[i]->release();
        
        for (int i = 0; i < totalCores; ++i)
            schedSemaphores[i]->acquire();

        checkCoreFinished();
        
        if (mode == "rr")
            checkRoundRobin();

        if (cpuCycle % (batchFreq+1) == 0 && makeProcesses.load()) {
            //if(latestProcessID < 10)
            generateProcess();
        }

        assignNewProcesses();

        
        cpuCycle++;

    }
}

void Scheduler::checkRoundRobin() {
	for (int i = 0; i < cores.size(); ++i) {
		auto core = cores[i];
		if (core->getProcess() && !core->isIdle()) {
            if (core->getQuantumCycleCounter() > 0 && core->getQuantumCycleCounter() % quantum_cycle == 0) {
                std::lock_guard<std::mutex> processLock(processMtx);
                std::lock_guard<std::mutex> coresLock(coresMtx);
                processQueue.push(core->getProcess());  
                ++freeCores;
                core->setProcessWait();
                core->assignProcess(nullptr);
                core->resetQuantumCounter();
            }
		}
	}
}


void Scheduler::assignNewProcesses() {
	for (int i = 0; i < cores.size(); ++i) {
		auto core = cores[i];
		if (core->isIdle() && !processQueue.empty()) {
            std::lock_guard<std::mutex> processLock(processMtx);
            std::lock_guard<std::mutex> coresLock(coresMtx);
            std::shared_ptr<Process> nextProc = processQueue.front();
            processQueue.pop();
            if (freeCores.load() > 0)
                --freeCores;
            core->resetQuantumCounter();
            core->assignProcess(nextProc);
            core->initializeProcess();
		}
	}
}

void Scheduler::checkCoreFinished() {
    for (int i = 0; i < cores.size(); ++i) {
        auto core = cores[i];
        if (core->getProcess() && core->isIdle()) {
            std::lock_guard<std::mutex> finishedLock(finishedMtx);
            std::lock_guard<std::mutex> coresLock(coresMtx);
            finishedQueue.push_back(core->getProcess());
            if(activeScreen == "")
                deleteScreen(core->getProcess()->getName());
            ++freeCores;
            core->assignProcess(nullptr);
            //cv.notify_one();
        }
    }   
}

void Scheduler::addProcess(std::string processName) {
    uint64_t instructionCount = (minInstructions == maxInstructions) ? minInstructions : getRandomInstructionCount(minInstructions, maxInstructions);
	auto newProcess = std::make_shared<Process>(latestProcessID, processName, instructionCount, addScreen(processName, latestProcessID, instructionCount));
	//newProcess->generateRandomCommands();

    newProcess->fixedSymbols();
    newProcess->fixedCommandSet();

    std::lock_guard<std::mutex> processLock(processMtx);
	processQueue.push(newProcess);
	latestProcessID++;
}

void Scheduler::generateProcess() {
    uint64_t instructionCount = (minInstructions == maxInstructions) ? minInstructions : getRandomInstructionCount(minInstructions, maxInstructions);
    std::string p_name = "p_" + std::to_string(latestProcessID);
    auto newProcess = std::make_shared<Process>(latestProcessID, p_name, instructionCount, addScreen(p_name, latestProcessID, instructionCount));

    newProcess->generateRandomCommands();
    //newProcess->fixedSymbols();
    //newProcess->fixedCommandSet();

    std::lock_guard<std::mutex> processLock(processMtx);
    processQueue.push(newProcess);
    latestProcessID++;

    //cv.notify_all();
}
