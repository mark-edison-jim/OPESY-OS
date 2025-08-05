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

        //for (int i = 0; i < totalCores; ++i)
        //    coreSemaphores[i]->release();
        //
        //for (int i = 0; i < totalCores; ++i)
        //    schedSemaphores[i]->acquire();

        for (int i = 0; i < totalCores; ++i) {
            coreSemaphores[i]->release();       
            schedSemaphores[i]->acquire();       
        }

        checkCoreFinished();

        if (mode == "rr")
            checkRoundRobin();

        if (makeProcesses.load() && cpuCycle % (batchFreq + 1) == 0) {
            //if(latestProcessID < 10)
            generateProcess();
            //std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        assignNewProcesses();

        //if(cpuCycle > 0 && cpuCycle % quantum_cycle == 0)
        //    memAcc.printStats(cpuCycle, memPerProcess);
        //std::this_thread::sleep_for(std::chrono::milliseconds(50));

        //if(cpuCycle % 100 == 0)
        cpuCycle++;
    }
}

void Scheduler::checkRoundRobin() {
    std::lock_guard<std::mutex> processLock(processMtx);
    std::lock_guard<std::mutex> waitingLock(waitingMtx);
    std::lock_guard<std::mutex> coresLock(coresMtx);
	for (int i = 0; i < cores.size(); ++i) {
		auto core = cores[i];
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (core->getProcess() && !core->isIdle()) {
            if (core->getQuantumCycleCounter() > 0 && core->getQuantumCycleCounter() % quantum_cycle == 0) {
                //memAcc.printStats(core->getQuantumCycleCounter(), memPerProcess);
                std::shared_ptr<Process> proc = core->getProcess();
                processQueue.push(proc);
                waitingQueue.push_back(proc);

                if (memAcc->getConfig()) {
                    memAcc->deallocateRR(proc->getPID(), proc->getNumPages());
                }

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
    std::lock_guard<std::mutex> waitingLock(waitingMtx);
    std::lock_guard<std::mutex> coresLock(coresMtx);
	for (int i = 0; i < cores.size(); ++i) {
		auto core = cores[i];
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (core->isIdle() && !processQueue.empty()) {

            std::shared_ptr<Process> nextProc = processQueue.front();
            processQueue.pop();
            waitingQueue.erase(waitingQueue.begin());
            if (memAcc->getConfig()) {
                if (memAcc->loadBStoPM(nextProc->getPID(), nextProc->getNumPages())) {
                    if (freeCores.load() > 0)
                        --freeCores;
                    core->resetQuantumCounter();
                    core->assignProcess(nextProc);
                    core->initializeProcess();
                }
                else {
                    processQueue.push(nextProc);
                    waitingQueue.push_back(nextProc);
                }
            }
            else {  
                if (freeCores.load() > 0)
                    --freeCores;
                core->resetQuantumCounter();
                core->assignProcess(nextProc);
                core->initializeProcess();
            }
                
		}
	}
}

void Scheduler::checkCoreFinished() {
    std::lock_guard<std::mutex> finishedLock(finishedMtx);
    std::lock_guard<std::mutex> coresLock(coresMtx);
    for (int i = 0; i < cores.size(); ++i) {
        auto core = cores[i];
        auto proc = core->getProcess();
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (proc && core->isIdle()) {
            finishedQueue.push_back(proc);
            if(activeScreen == "" && !core->getProcessAbrupt() && proc->getState() != 4)
                deleteScreen(proc->getName());
            //memAcc.deallocateMemory(proc->getName(), memPerProcess, cpuCycle);
            core->flushProcessMemory();
            ++freeCores;
            core->assignProcess(nullptr);
            //cv.notify_one();
        }
    }   
}

void Scheduler::addProcess(std::string processName, uint16_t memPerProcess) {
    uint64_t instructionCount = (minInstructions == maxInstructions) ? minInstructions : getRandomInstructionCount(minInstructions, maxInstructions);
	auto newProcess = std::make_shared<Process>(latestProcessID, processName, instructionCount, addScreen(processName, latestProcessID, instructionCount), memPerProcess, memPerBlock, memAcc);
	
    //newProcess->generateRandomCommands();
    newProcess->fixedCommandSet();

    //newProcess->fixedSymbols();

    std::lock_guard<std::mutex> waitingLock(waitingMtx);
    std::lock_guard<std::mutex> processLock(processMtx);
    waitingQueue.push_back(newProcess);
    processQueue.push(newProcess);
	latestProcessID++;
}

void Scheduler::addProcess(std::string processName, uint16_t memPerProcess, std::string commandsList) {
    auto newProcess = std::make_shared<Process>(latestProcessID, processName, 0, addScreen(processName, latestProcessID, 0), memPerProcess, memPerBlock, memAcc);

    //newProcess->generateRandomCommands();
    newProcess->fixedCommandSet(commandsList);

    //newProcess->fixedSymbols();


    std::lock_guard<std::mutex> waitingLock(waitingMtx);
    std::lock_guard<std::mutex> processLock(processMtx);
    waitingQueue.push_back(newProcess);
    processQueue.push(newProcess);
    latestProcessID++;
}

void Scheduler::generateProcess() {
    uint64_t instructionCount = (minInstructions == maxInstructions) ? minInstructions : getRandomInstructionCount(minInstructions, maxInstructions);
    uint16_t memPerProcess = (minMemPerProcess == maxMemPerBlock) ? minMemPerProcess : getRandomMemory(minMemPerProcess, maxMemPerBlock);

    std::string p_name = "p_" + std::to_string(latestProcessID);
    auto newProcess = std::make_shared<Process>(latestProcessID, p_name, instructionCount, addScreen(p_name, latestProcessID, instructionCount), memPerProcess, memPerBlock, memAcc);
    
    newProcess->generateRandomCommands();
    //newProcess->fixedCommandSet();
    //newProcess->fixedSymbols();

    std::lock_guard<std::mutex> waitingLock(waitingMtx);
    std::lock_guard<std::mutex> processLock(processMtx);
    waitingQueue.push_back(newProcess);
    processQueue.push(newProcess);
    latestProcessID++;

    //cv.notify_all();
}
