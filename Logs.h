#pragma once
#include <fstream>
class Logs
{
private:
	const std::string fileName = "log.txt"; 
	std::ofstream logFile;

	Logs(const Logs&) = delete;
	Logs& operator=(const Logs&) = delete;

	static Logs& instance() {
		static Logs logManager;
		return logManager;
	}

	Logs() : logFile(fileName, std::ios::app) {}

	~Logs() {
		logFile.close();
	}

public:
	static void appendLine();
};

