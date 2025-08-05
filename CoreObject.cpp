#include "CoreObject.hpp"
#include <iostream>
#include <thread> 
#include <chrono> 

void CoreObject::run() {
    while (true) {
        coreSemaphore->acquire();
        std::lock_guard<std::mutex> lock(processMutex);
        if (process && coreCycle > 0 && coreCycle % (delay+1) == 0) {
            process->runCommand();
            activeCoreCycle++;
            //coreCycle = 1;
        }
		if (process && (process->getState() == 3 || process->getState() == 4))
            processFinished = true;

        if (process && process->getState() == 4)
            processAbrupted = true;

        incrementQuantumCycleCounter();
        coreCycle++;
        schedSemaphore->release();
    }
}

void CoreObject::initializeProcess() {
	if (process) {
		process->beginProcess(coreID);
		processFinished = false;
	}
}

//std::thread processThread([p, this]() {
    //assignProcess(p);
//});

//processThread.join(); 