#include "PrintCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "utils.hpp"

PrintCommand::PrintCommand(int pid, std::string& text, std::shared_ptr<std::unordered_map<std::string, uint16_t>> symbolTable) : ICommand(PRINT, pid, symbolTable) {
	this->text = text;
}

void PrintCommand::execute(int cpuCoreID) {
	//ICommand::execute();
	
	

	//std::string preText = getTime() + "  Core:" + std::to_string(cpuCoreID) + " ";

	//std::string fileName = "file_" + std::to_string(pid) + ".txt";
	////std::string path = "C:\\Users\\admin\\Desktop\\Mark\\Mark's Work\\College\\3rd Year 3rd Term\\CSOPESY\\TextOutputs\\" + fileName;
	//std::string path = "TextOutputs/" + fileName;

	//std::ofstream MyFile(path, std::ios_base::app);

	//if (MyFile.is_open()) {
	//	MyFile << preText + text << std::endl;
	//}
	//MyFile.close();
}