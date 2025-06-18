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
    int totalLines;
    std::string creationDate;
    std::deque<std::string> logs;
    std::vector<std::string> commandHistory;
    std::vector<std::string> outputHistory;
    bool processFinished = false;

public:
    Screen() = default;

    Screen(const std::string& name, int pid, int totalLines) : name(name), pid(pid), totalLines(totalLines) {
        creationDate = getTime();
    }

    std::string getName() const;
    std::string getCreationDate() const;

    bool getProcessFinished() const {
        return processFinished;
    }

	int getPid() const {
		return pid;
	}

	int getCurrentLine() const {
		return currentLine;
	}

	int getTotalLines() const {
		return totalLines;
	}

    const std::vector<std::string> getCommands() const;
    const std::vector<std::string> getOutputs() const;

    std::deque<std::string> getLogs() const {
        return logs;
    }

    void update(const std::deque<std::string>&, int);
    
    void finish(const std::deque<std::string>&);

    void addCommand(const std::string& cmd, const std::string& output);


    void clearHistory();
};
