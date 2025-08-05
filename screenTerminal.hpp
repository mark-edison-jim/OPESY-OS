#pragma once

#include <string>
#include <vector>
#include <deque>
#include "utils.hpp"

class Screen {
private:
    std::string name;
    int pid;
    int currentLine = 0;
    uint64_t totalLines;
    std::string creationDate;
    std::deque<std::string> logs;
    std::vector<std::string> commandHistory;
    std::vector<std::string> outputHistory;
    std::string invalidAddress = "";
    bool processFinished = false;
    bool invalidMem = false;
    std::string timeInvalidOccured;

public:
    Screen() = default;

    Screen(const std::string& name, int pid, uint64_t totalLines) : name(name), pid(pid), totalLines(totalLines) {
        creationDate = getTime();
    }

    std::string getName() const;
    std::string getCreationDate() const;

    bool getProcessFinished() const {
        return processFinished;
    }

    bool getProcessAbrupted() const {
        return invalidMem;
    }

    std::string getTimeInvalid() {
        return timeInvalidOccured;
    }

    std::string getInvalidAddress() {
        return invalidAddress;
    }

    size_t getPid() const {
		return pid;
	}

	int getCurrentLine() const {
		return currentLine;
	}

	uint64_t getTotalLines() const {
		return totalLines;
	}

    void setTotalLines(int lines) {
        totalLines = lines;
    }

    const std::vector<std::string> getCommands() const;
    const std::vector<std::string> getOutputs() const;

    std::deque<std::string> getLogs() const {
        return logs;
    }

    void update(const std::deque<std::string>&, int);
    
    void finish(const std::deque<std::string>&);

    void invalidFinish(const std::deque<std::string>& finalLogs, std::string);

    void addCommand(const std::string& cmd, const std::string& output);


    void clearHistory();
};
