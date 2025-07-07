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

        for (int i = 0; i < totalCores; ++i)
            coreSemaphores[i]->release();
        
        for (int i = 0; i < totalCores; ++i)
            schedSemaphores[i]->acquire();


        checkCoreFinished();

        if (mode == "rr")
            checkRoundRobin();

        if (cpuCycle > 0 && cpuCycle % (batchFreq + 1) == 0 && makeProcesses.load()) {
            if(latestProcessID < 21)
                generateProcess();
        }
        assignNewProcesses();

        //if(cpuCycle % 100 == 0)
            //std::this_thread::sleep_for(std::chrono::microseconds(100));
        cpuCycle++;
    }
}

void Scheduler::checkRoundRobin() {
    std::lock_guard<std::mutex> processLock(processMtx);
    std::lock_guard<std::mutex> coresLock(coresMtx);
	for (int i = 0; i < cores.size(); ++i) {
		auto core = cores[i];
		if (core->getProcess() && !core->isIdle()) {
            if (core->getQuantumCycleCounter() > 0 && core->getQuantumCycleCounter() % quantum_cycle == 0) {
                //memAcc.printStats(cpuCycle, memPerProcess);
                processQueue.push(core->getProcess());
                ++freeCores;
                core->setProcessWait();
                core->assignProcess(nullptr);
                //core->resetQuantumCounter();screen -ls
            }
		}
	}
}

void Scheduler::assignNewProcesses() {
    std::lock_guard<std::mutex> processLock(processMtx);
    std::lock_guard<std::mutex> coresLock(coresMtx);
	for (int i = 0; i < cores.size(); ++i) {
		auto core = cores[i];
        if (core->isIdle() && !processQueue.empty()) {
            std::shared_ptr<Process> nextProc = processQueue.front();
            processQueue.pop();
            if (memAcc.checkProcInMemory(nextProc->getName()) || memAcc.allocateMemory(nextProc->getName(), memPerProcess)) {
                //memAcc.printStats(cpuCycle, memPerProcess);
                if (freeCores.load() > 0)
                    --freeCores;
                core->resetQuantumCounter();
                core->assignProcess(nextProc);
                core->initializeProcess();
            }else
                processQueue.push(nextProc); 
		}
	}
}

void Scheduler::checkCoreFinished() {
    std::lock_guard<std::mutex> finishedLock(finishedMtx);
    std::lock_guard<std::mutex> coresLock(coresMtx);
    for (int i = 0; i < cores.size(); ++i) {
        auto core = cores[i];
        auto proc = core->getProcess();
        if (proc && core->isIdle()) {
            finishedQueue.push_back(proc);
            if(activeScreen == "")
                deleteScreen(proc->getName());
            //memAcc.printStats(cpuCycle, memPerProcess);
            memAcc.deallocateMemory(proc->getName(), memPerProcess);
            ++freeCores;
            core->assignProcess(nullptr);
            //cv.notify_one();
        }
    }   
}

void Scheduler::addProcess(std::string processName) {
    uint64_t instructionCount = (minInstructions == maxInstructions) ? minInstructions : getRandomInstructionCount(minInstructions, maxInstructions);
	auto newProcess = std::make_shared<Process>(latestProcessID, processName, instructionCount, addScreen(processName, latestProcessID, instructionCount), memPerProcess);
	newProcess->generateRandomCommands();

    //newProcess->fixedSymbols();
    //newProcess->fixedCommandSet();

    std::lock_guard<std::mutex> processLock(processMtx);
	processQueue.push(newProcess);
	latestProcessID++;
}

void Scheduler::generateProcess() {
    uint64_t instructionCount = (minInstructions == maxInstructions) ? minInstructions : getRandomInstructionCount(minInstructions, maxInstructions);
    std::string p_name = "p_" + std::to_string(latestProcessID);
    auto newProcess = std::make_shared<Process>(latestProcessID, p_name, instructionCount, addScreen(p_name, latestProcessID, instructionCount), memPerProcess);

    newProcess->generateRandomCommands();
    //newProcess->fixedSymbols();
    //newProcess->fixedCommandSet();

    std::lock_guard<std::mutex> processLock(processMtx);
    processQueue.push(newProcess);
    latestProcessID++;

    //cv.notify_all();
}
