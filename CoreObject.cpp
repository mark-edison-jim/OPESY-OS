#include "CoreObject.hpp"
#include <iostream>
#include <thread> 
#include <chrono> 

void CoreObject::run() {
    while (true) {
        coreSemaphore->acquire();
        if (process && coreCycle % (delay+1) == 0) {
            process->runCommand();
            incrementQuantumCycleCounter();
            //coreCycle = 1;
        }
		if (process && process->getState() == 3)
            processFinished = true;
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