#include "Logs.h"
#include "MySQLManager.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>

std::string Logs::getCurrentTime(std::int64_t timestamp)
{
	std::time_t in_time_t = (timestamp == 0)
		? std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
		: static_cast<std::time_t>(timestamp);

	struct tm timeinfo;
	localtime_s(&timeinfo, &in_time_t);

	std::stringstream ss;
	ss << std::put_time(&timeinfo, "%Y-%m-%d %X");
	return ss.str();
}

int64_t Logs::getCurrentTimeStamp()
{
	auto now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
	return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

std::string 
Logs::parseOperation	(CrudOperation& op, 
						std::string& user, 
						std::string& equipment, 
						int equipmentID,
						int64_t timestamp, 
						std::string& parsedTime) 
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

	if (MySQLManager::logOp(opToStr, user, equipmentID, timestamp, parsedTime) == MySQLResult::InternalServerError)
		return "";

	if (op != CrudOperation::Read)
		return "User `" + user + "` performed `" + opToStr + "` on equipment `" + equipment +
			"` #`" + std::to_string(equipmentID) + "`";
	else 
		return "User `" + user + "` viewed equipment category `" + equipment + "`";
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

	int64_t currentTimeStamp = getCurrentTimeStamp();
	std::string currentTime = getCurrentTime(currentTimeStamp);

	std::string line = '[' + currentTime + ']';
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
		{
			std::string operationParsed = Logs::parseOperation(op, user, equipement, equipmentID, currentTimeStamp, currentTime);
			if (operationParsed.empty()) {
				logLine(Type::Error, CrudOperation::Read, "", "", -1, "Error while parsing operation logs");
				return;
			}
			line += " [MANAGEMENT] ";
			line += operationParsed;
			break;
		}
		case Type::Error:
			line += " [ERROR] ";
			line += error;
			break;
	}

	if (file.is_open()) {
		file << line << std::endl;
	}
}
