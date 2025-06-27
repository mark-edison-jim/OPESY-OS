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

        //if (cpuCycle % 1000 == 0)
        //    std::this_thread::sleep_for(std::chrono::milliseconds(1));
        cpuCycle++;
        std::this_thread::sleep_for(std::chrono::microseconds(3));
        //if (latestProcessID == 1) {
        //    for (int i = 0; i < 1; i++) {

        //    }
        //}
        // 
        //P1=5
        //P2=5
        //dpe=1
        //qc=2

        //5 5 4
        //    5 5 4
        //    P
        //0 1 2 3
        //X R X R
        //



        //if (latestProcessID >= numProcesses &&
        //    processQueue.empty() && 
        //    finishedQueue.size() == numProcesses){
        //    break;
        //}

    }
}

void Scheduler::checkRoundRobin() {
	for (int i = 0; i < cores.size(); ++i) {
		auto core = cores[i];
		if (core->getProcess() && !core->isIdle()) {
            if (core->getQuantumCycleCounter() > 0 && core->getQuantumCycleCounter() % quantum_cycle == 0) {
                std::lock_guard<std::mutex> lock(mtx);
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
            std::lock_guard<std::mutex> lock(finishedMtx);
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
	newProcess->generateRandomCommands();
    std::lock_guard<std::mutex> lock(mtx);
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

    std::lock_guard<std::mutex> lock(mtx);
    processQueue.push(newProcess);
    latestProcessID++;

    //cv.notify_all();
}
