#include "Logs.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>

std::string Logs::getCurrentTime()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	struct tm timeinfo;
	localtime_s(&timeinfo, &in_time_t);

	std::stringstream ss;
	ss << std::put_time(&timeinfo, "%Y-%m-%d %X");
	return ss.str();
}

std::string Logs::parseOperation(CrudOperation& op, std::string& user, std::string& equipment, int equipmentID)
{
	std::string opToStr;

	switch (op) {
	case CrudOperation::Create:
		opToStr = "Add";
		break;
	case CrudOperation::Read:
		opToStr = "View";
		break;
	case CrudOperation::Update:
		opToStr = "Edit";
		break;
	case CrudOperation::Delete:
		opToStr = "Delete";
		break;
	}

	return "User `" + user + "` performed `" + opToStr + "` on equipment `" + equipment +
		"` with ID `" + std::to_string(equipmentID) + "`";
}

void Logs::logLine(
	Type type, 
	CrudOperation op, 
	std::string user, 
	std::string equipement, 
	int equipmentID, 
	std::string error
){

	std::ofstream& file = instance().logFile;

	std::string line = '[' + getCurrentTime() + ']';
	switch (type) {
		case Type::Start:
			line += " [START] ";
			line += "Server Started Successfully.";
			break;
		case Type::Stop:
			line += " [STOP] ";
			line += "Server Stopped Successfully.";
			break;
		case Type::Operation:
			line += " [MANAGEMENT] ";
			line += Logs::parseOperation(op, user, equipement, equipmentID);
			break;
		case Type::Error:
			line += " [ERROR] ";
			line += error;
			break;
	}

	if (file.is_open()) {
		file << line << std::endl;
	}
}
