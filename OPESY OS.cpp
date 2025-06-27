//#include <curses.h>
#include <vector>
#include <string>
#include <thread>
#include <sstream>
#include <chrono>
#include <iostream>
#include <windows.h>
#include <sstream>
#include <conio.h>
#include <iomanip>
#include "utils.hpp"
//#include "Cube.hpp"
#include "Scheduler.hpp"
#include <atomic>
#include <iomanip>
#include <fstream>
#include <chrono>

using namespace std::chrono;

std::atomic<bool> schedulerRunning = false;
std::thread schedulerThread;
std::string command = "";
std::vector<std::string> cmd_hist;
std::vector<std::string> output_hist;
auto lastInputPoll = steady_clock::now();
std::vector<std::string> marquee_cmd_hist;

int hist_inc = 0;

bool isScreening = false;
std::string activeTerminal = "main";
//std::string activeScreen;

bool exitOS = false;

bool isInitialized = false;
std::shared_ptr<Scheduler> globalScheduler;

int numCPU = 0;
std::string scheduler;
int quantumCycles = 0;
int batchFreq = 0;
uint64_t minInstructions = 0;
uint64_t maxInstructions = 0;
int delayPerExec = 0;


void initializeFunc(std::string* action) {
    *action = commandMsg("'initialize' command recognized. Doing something.");
	isInitialized = true;
    std::string line;
    std::ifstream config("config.txt");
    while (getline(config, line)) {
        std::istringstream iss(line);
        std::string key;
        if (!(iss >> key)) continue;

        if (key == "num-cpu") {
            iss >> numCPU;
        }
        else if (key == "scheduler") {
            std::string sched;
            iss >> std::quoted(sched);
            scheduler = sched;
        }
        else if (key == "quantum-cycles") {
            iss >> quantumCycles;
        }
        else if (key == "batch-process-freq") {
            iss >> batchFreq;
        }
        else if (key == "min-ins") {
            iss >> minInstructions;
        }
        else if (key == "max-ins") {
            iss >> maxInstructions;
        }
        else if (key == "delay-per-exec") {
            iss >> delayPerExec;
        }
    }

	config.close();
    
    globalScheduler = std::make_shared<Scheduler>(numCPU, 10, minInstructions, maxInstructions, delayPerExec, batchFreq, scheduler, quantumCycles);
    std::thread schedulerThread([scheduler = globalScheduler]() {
        globalScheduler->fcfs();
        });
    

    schedulerThread.detach();

}

void screenTerminal() {
    char ch;
    if (_kbhit()) {
        ch = _getch();

        if ((int)ch == 8) command = command.substr(0, command.length() - 1);
        else if ((int)ch == 127) command = "";
        else if ((int)ch == 13) {
            hist_inc = 0;
            if (command == "exit") {
                auto screen = globalScheduler->getScreen();
                if(screen->getProcessFinished())
                    globalScheduler->deleteScreen(screen->getName());
                globalScheduler->setActiveScreen("");

                activeTerminal = "main";
                hist_inc = 0;
            }
            else if (command == "process-smi") {
                std::ostringstream out;

				std::shared_ptr<Screen> screen = globalScheduler->getScreen();

                bool unFinished = !screen->getProcessFinished();

                std::deque<std::string> logs = screen->getLogs();
                out << "Process name: " << screen->getName() << std::endl;
                out << "ID: " << screen->getPid() << std::endl;
                out << "Logs:" << std::endl;
                for (const std::string& log : logs) {
                    out << log << std::endl;
                }
                if (unFinished) {
                    out << std::endl << "Current instrucation line: " << screen->getCurrentLine() << std::endl;
                    out << "Lines of code: " << screen->getTotalLines() << std::endl << std::endl;
                }
                else{
                    out << std::endl << "Finished!" << std::endl << std::endl;
                }
                globalScheduler->addCommandToScreen(command, out.str());
            }
            else {
                globalScheduler->addCommandToScreen(command, commandMsg("Did something..."));
            }
            command = "";
        }
        else if ((int)ch >= 32 && (int)ch <= 126) {
            command += ch;
        }
    }

	std::string activeScr = globalScheduler->getActiveScreen();
    auto scr = globalScheduler->getScreen();
    if (scr) {
        const std::vector<std::string> cmds = scr->getCommands();
        const std::vector<std::string> outs = scr->getOutputs();

        //std::cout << "Process Name: " << scr.getName() << std::endl;
        //std::cout << "Total line of instructions: " << scr.totalLines() << std::endl;
        //std::cout << "Creation Date: " << scr.getCreationDate() << std::endl << std::endl;

        for (int i = 0; i < cmds.size(); ++i) {
            std::cout << activeScr << ":\\> " << cmds[i] << std::endl;
            std::cout << outs[i] << std::endl;
        }
    }

    std::cout << activeScr << ":\\> " << command << std::endl << std::endl;
}


std::ostringstream getProcessStats() {
    std::ostringstream out;
    std::ostringstream runningPOut;
    std::ostringstream finishedPOut;

    int coresUsed = 0;
	std::vector<std::shared_ptr<CoreObject>> cores = globalScheduler->getCoresArray();
    for (std::shared_ptr<CoreObject> core : cores) {
        std::shared_ptr<Process> p = core->getProcess();
        if (p) {
            coresUsed++;
            runningPOut << std::left << std::setw(12) << p->getName() <<
                std::setw(30) << p->getDate() <<
                std::setw(12) << "Core: " + std::to_string(p->getCpuCoreID()) <<
                p->getCommandIndex() << "/" << p->getLinesOfCode() << std::endl;
        }
    }

    int freeCores = numCPU - coresUsed;
    double cputil = (static_cast<double>(coresUsed) / numCPU) * 100.0;

    for (std::shared_ptr<Process> p : globalScheduler->getFinishedQueue()) {
        finishedPOut << std::left << std::setw(12) << p->getName() <<
            std::setw(30) << p->getDate() <<
            std::setw(12) << "Finished" <<
            p->getCommandIndex() << "/" << p->getLinesOfCode() << std::endl;
    }

    out << "CPU Utilization: " << std::fixed << std::setprecision(2) << cputil << " %" << std::endl;
    out << "Cores Used: " + std::to_string(coresUsed) << std::endl;
    out << "Cores Available: " + std::to_string(freeCores) << std::endl << std::endl;
    out << "+-----------------------------------------------------------------------------------------+" << std::endl;
    out << "Running Processes:" << std::endl;
    out << runningPOut.str() << std::endl;
    out << "Finished Processes:" << std::endl;
    out << finishedPOut.str() << std::endl;
    out << "+-----------------------------------------------------------------------------------------+" << std::endl << std::endl;

    return out;
}

void screenFunc(std::string* action, std::vector<std::string> cmdTokens) {
    //*action = commandMsg("'screen' command recognized. Doing something.");
    if (cmdTokens.size() == 3) {
        std::string mode = cmdTokens[1];
        std::string name = cmdTokens[2];
        bool nameExists = globalScheduler->findScreen(name);
        bool validMode = (mode == "-s" || mode == "-r" || mode == "-ls");

        if (!validMode) {
            *action = commandMsg("Invalid mode. Use -s or -r.");
            return;
        }

        if (mode == "-s") {
            if (nameExists) {
                *action = commandMsg("<screen." + name + "> already exists...");
                return;
            }
			globalScheduler->addProcess(name);

            *action = commandMsg("Switching to <screen." + name + ">...");
        }
        else if (mode == "-r") {
            if (!nameExists) {
                *action = commandMsg("<screen." + name + "> does not exist...");
                return;
            }
            *action = commandMsg("Switching to <screen." + name + ">...");
        }

		globalScheduler -> setActiveScreen(name);
        activeTerminal = "screen";
        hist_inc = 0;
    }
    else if (cmdTokens.size() == 2) {
        std::string mode = cmdTokens[1];
        if (mode == "-ls") {

            *action = getProcessStats().str();
        }
        else {
            *action = commandMsg("Invalid screen command. Use 'screen -s <name>' | 'screen -r <name>' | 'screen -ls'.");
        }
    }
    else {
        *action = commandMsg("Invalid screen command. Use 'screen -s <name>' | 'screen -r <name>' | 'screen -ls'");
    }
}

void testThreadFunc() {
    while (true) {
        std::cout << "Scheduler thread running..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    //std::cout << "Scheduler thread exiting..." << std::endl;
}

void schedulerStartFunc(std::string* action) {
    if (schedulerRunning) {
        *action = commandMsg("Scheduler already running.");
        return;
    }
	globalScheduler->makeProcess();
    *action = commandMsg("Scheduler started...");
    schedulerRunning = true;
    //schedulerThread = std::thread(testThreadFunc);
}

void schedulerStopFunc(std::string* action) {
    if (!schedulerRunning) {
        *action = commandMsg("Scheduler not running.");
        return;
    }
	globalScheduler->stopMakingProcess();
    schedulerRunning = false;
    *action = commandMsg("Scheduler stopped...");
}

void reportUtilFunc(std::string* action) {

    *action = commandMsg("'report-util' command recognized. Doing something.");

    std::string path = "report_util.txt";

    std::ofstream MyFile(path);

    if (MyFile.is_open()) {
        MyFile << getProcessStats().str() << std::endl;
    }
    MyFile.close();
}

void handleInput(std::vector<std::string> cmdTokens) {
    std::string action;
    std::vector<std::string> validCommands = { "scheduler-start", "scheduler-stop", "report-util", "screen", "marquee" };
    std::string tempCommand = cmdTokens[0];
    if (tempCommand == "initialize") {
        initializeFunc(&action);
    }
    else if (std::find(validCommands.begin(), validCommands.end(), tempCommand) != validCommands.end()) {
        if (isInitialized) {
            if (tempCommand == "screen") {
                screenFunc(&action, cmdTokens);
            }
            else if (tempCommand == "scheduler-start") {
                schedulerStartFunc(&action);
            }
            else if (tempCommand == "scheduler-stop") {
                schedulerStopFunc(&action);
            }
            else if (tempCommand == "report-util") {
                reportUtilFunc(&action);
            }
            else if (tempCommand == "marquee") {
				if (activeTerminal == "main") {
					activeTerminal = "marquee";
					command = "";
				}
				action = commandMsg("Switched to MARQUEE_CONSOLE...");
			}
			else
				action = commandMsg("Command recognized but not implemented yet...");
        }
        else
            action = commandMsg("Please initialize first...");
    }
    else if (tempCommand == "clear") {
        cmd_hist.clear();
        output_hist.clear();
    }
    else if (tempCommand == "nvidia-smi") {
        action = printGPUSummary();
        std::cout << action << std::endl;
    }
    else {
        action = commandMsg("Invalid Command...");
    }
    if (!action.empty())
        output_hist.push_back(action);
    command = "";
}

COORD getCursorPosition() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD pos = { 0, 0 };
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        pos = csbi.dwCursorPosition;
    }
    return pos;
}

void setCursorPosition(int x, int y) {
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}   

void marqueeActivity() {
    auto now = steady_clock::now();
    char ch;
    if (duration_cast<milliseconds>(now - lastInputPoll).count() >= pollInterval) {
        if (_kbhit())
        {
            ch = _getch();
            if ((int)ch == 8) // Backspace
                command = command.substr(0, command.length() - 1);
            else if ((int)ch == 127) // Ctrl + Backspace
                command = "";
            else if ((int)ch == 13) {// Enter
                hist_inc = 0;
                std::vector<std::string> cmdTokens = split(command, ' ');
                if (cmdTokens[0] == "exit") {
                    activeTerminal = "main";
                    command = "";
                }
                else {
                    marquee_cmd_hist.push_back(command);
                    command = "";
                }
            }
            else if ((int)ch >= 32 && (int)ch <= 126) // Printable characters
                command += ch;
        }
        lastInputPoll = now;
    }
        std::cout << std::endl;
        std::cout << "Enter a command for MARQUEE_CONSOLE: " << command;
        COORD savedPos = getCursorPosition();
        std::cout << std::endl;
        for (int i = 0; i < marquee_cmd_hist.size(); i++) {
            std::cout << "Command processed for MARQUEE_CONSOLE: " << marquee_cmd_hist[i] << std::endl;
        }
        setCursorPosition(savedPos.X, savedPos.Y);

}

void mainTerminal() {
    char ch;
    if (_kbhit())
    {
        ch = _getch();
		if ((int)ch == 27) // Escape
            exitOS = true;
		else if ((int)ch == 8) // Backspace
            command = command.substr(0, command.length() - 1);
		else if ((int)ch == 127) // Ctrl + Backspace
            command = "";
        else if ((int)ch == 13) {// Enter
            hist_inc = 0;
            std::vector<std::string> cmdTokens = split(command, ' ');
            if (cmdTokens[0] == "exit") {
                exitOS = true;
                globalScheduler->stopOS();
            }
            else {
                cmd_hist.push_back(command);
                handleInput(cmdTokens);
            }
        }
		else if ((int)ch >= 32 && (int)ch <= 126) // Printable characters
            command += ch;
    }

    std::cout << std::endl;

    for (int i = 0; i < cmd_hist.size(); i++) {
        std::cout << "root:\\> " << cmd_hist[i] << std::endl;
        std::cout << output_hist[i] << std::endl;
    }

    std::cout << "root:\\> " << command << std::endl;
    //refresh();
}


int main() {

    //initscr();
    //noecho();
    //curs_set(0);
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = FALSE; // Set the cursor visibility to false
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);

    int width = getConsoleWidth();
    int height = getConsoleHeight();
    system("cls");
    //Cube my_cube = Cube(15, 30, 20, 1);
    //my_cube.set_speed(1);
    //my_cube.set_background(' ');

    for (int i = 0; i < 50; ++i) {
        std::cout << "Warming up..." << std::endl;
        Sleep(10);
        system("cls");
    }

    int i = 0;
    int offset = 0;
    while (!exitOS) {
        system("cls");

        if (activeTerminal == "main") {
            printSlider(offset);
            mainTerminal();

		}
		else if (activeTerminal == "marquee") {
            //my_cube.printCube();
            printMarquee();
			marqueeActivity();
		}
        else if (activeTerminal == "screen") {
			screenTerminal();
        }

        if (i % marqueeRate == 0)
            offset += 1;
        i += 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(refreshRate));
    }
    // near end of main()
    exitOS = true;
    schedulerRunning = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    //endwin();
    return 0;
}
