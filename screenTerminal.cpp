#include "screenTerminal.hpp"
#include "utils.hpp"


std::string Screen::getName() const {
    return name;
}

std::string Screen::getCreationDate() const {
    return creationDate;
}

void Screen::update(const std::deque<std::string>& newLogs, int currLine) {
    logs = newLogs;
	currentLine = currLine;
}

void Screen::finish(const std::deque<std::string>& finalLogs){
    logs = finalLogs;
    processFinished = true;
}

void Screen::addCommand(const std::string& cmd, const std::string& output) {
    commandHistory.push_back(cmd);
    outputHistory.push_back(output);
}

const std::vector<std::string> Screen::getCommands() const {
    return commandHistory;
}

const std::vector<std::string> Screen::getOutputs() const {
    return outputHistory;
}

void Screen::clearHistory() {
    commandHistory.clear();
    outputHistory.clear();
}
