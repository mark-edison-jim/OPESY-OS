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

    while (true) {
        //std::unique_lock<std::mutex> lock(mtx);
        //cv.wait(lock, [&]() {   
        //    return freeCores > 0 || !processQueue.empty();
        //    });

        checkCoreFinished();

        if (mode == "rr") 
			checkRoundRobin();
        
        assignNewProcesses();

        for (int i = 0; i < totalCores; ++i)
            coreSemaphores[i]->release();

        for (int i = 0; i < totalCores; ++i)
            schedSemaphores[i]->acquire();


        if (cpuCycle % (batchFreq+1) == 0 && makeProcesses.load()) {
            //if(latestProcessID < numProcesses)
            generateProcess();
        }
        
        //if (latestProcessID == 1) {
        //    for (int i = 0; i < 1; i++) {

        //    }
        //}

        //if (latestProcessID >= numProcesses &&
        //    processQueue.empty() && 
        //    finishedQueue.size() == numProcesses){
        //    break;
        //}
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cpuCycle++;
    }
}

void Scheduler::checkRoundRobin() {
	for (int i = 0; i < cores.size(); ++i) {
		auto core = cores[i];
		if (core->getProcess() && !core->isIdle()) {
            if (core->getQuantumCycleCounter() >= quantum_cycle / (execDelay + 1)) {
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

	auto newProcess = std::make_shared<Process>(latestProcessID, processName, numCommands, addScreen(processName, latestProcessID, numCommands));
	newProcess->generateRandomCommands();
    std::lock_guard<std::mutex> lock(mtx);
	processQueue.push(newProcess);
	latestProcessID++;
}

void Scheduler::generateProcess() {
    std::string p_name = "screen_0" + std::to_string(latestProcessID);
    auto newProcess = std::make_shared<Process>(latestProcessID, p_name, numCommands, addScreen(p_name, latestProcessID, numCommands));

	newProcess->generateRandomCommands();

    std::lock_guard<std::mutex> lock(mtx);
    processQueue.push(newProcess);
    latestProcessID++;

    //cv.notify_all();
}
