#include "PrintCommand.hpp"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>

PrintCommand::PrintCommand(int pid, std::string& text) : ICommand(PRINT, pid) {
	this->text = text;
}

void PrintCommand::execute() {
	//ICommand::execute();
	


	std::string fileName = "file_" + std::to_string(pid) + ".txt";
	std::string path = "C:\\Users\\admin\\Desktop\\Mark\\Mark's Work\\College\\3rd Year 3rd Term\\CSOPESY\\TextOutputs\\" + fileName;

	std::ofstream MyFile(path, std::ios_base::app);

	if (MyFile.is_open()) {
		MyFile << text << std::endl;
	}
	MyFile.close();
}