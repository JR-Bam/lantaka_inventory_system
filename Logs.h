#pragma once
#include <fstream>
class Logs
{
public:
	enum class Type {
		Start,
		Stop,
		Error,
		Operation
	};

	enum CrudOperation {
		Create,
		Read,
		Update,
		Delete
	};

	static void logLine(
		Type type,
		CrudOperation op = CrudOperation::Read,
		std::string user = std::string(),
		std::string equipment = std::string(),
		int equipmentID = 1,
		std::string error = std::string()
	);

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

	// From: https://stackoverflow.com/questions/17223096/outputting-date-and-time-in-c-using-stdchrono
	static std::string getCurrentTime(std::int64_t timestamp = 0);
	static int64_t getCurrentTimeStamp();
	static std::string parseOperation(CrudOperation& op, std::string& user, std::string& equipment, int equipmentID, int64_t timestamp, std::string& parsedTime);
};

