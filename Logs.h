/*****************************************************************//**
 * \file   Logs.h
 * \brief  File used for logging events such as start, stop, error, and operations.
 * 
 * \date   November 2024
 *********************************************************************/
#pragma once
#include <fstream>

/**
 * \class Logs Logs.h "Logs.h"
 * @brief Singleton class to log events.
 * 
 * This class is structured to be singleton solely because the main purpose is to house functions used for logging.
 * Logging depends on an std::ofstream file that needs to be closed on exit. Therefore a RAII approach is implemented.
 */
class Logs
{
public:
	/**
	 * @brief The type of message to be logged.
	 */
	enum class Type {
		/// For when the program starts
		Start,
		/// For when the program ends (properly)
		Stop,
		/// For when an error occurs internally
		Error,
		/// For when a CRUD operation is being executed
		Operation
	};

	/**
	 * @brief The type of CRUD operation that's being executed.
	 */
	enum class CrudOperation {
		Create,
		Read,
		Update,
		Delete
	};

	/**
	 * @brief Used to append a line in the logs file.
	 * 
	 * Given the Logs::Type passed, it will only print out the following params.
	 * 1. Logs::Type::Start - type
	 * 2. Logs::Type::Stop - type
	 * 3. Logs::Type::Error - type, error
	 * 4. Logs::Type::Operation - type, op, user, equipment, equipmentID
	 * 
	 * \param type the type of log
	 * \param op the CRUD operation to display
	 * \param user the username of the user that executed the CRUD operation
	 * \param equipment name of the equipment involved in the CRUD operation. For CrudOperation::View, this should be the name of the Main Category of equipment
	 * \param equipmentID id of the equipment
	 * \param error information about the error that occured
	 */
	static void logLine		(Type type,
							 CrudOperation op = CrudOperation::Read,
							 std::string user = std::string(),
							 std::string equipment = std::string(),
							 int equipmentID = 1,
							 std::string error = std::string());

private:
	const std::string fileName = "log.txt"; /// Name of the text file logs will output to
	std::ofstream logFile; /// The output file object

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

	/**
	 * @brief Get the current time stamp and parse it as a string with the format "%Y-%m-%d %X".
	 * 
	 * Taken from: https://stackoverflow.com/questions/17223096/outputting-date-and-time-in-c-using-stdchrono
	 * 
	 * \param timestamp an integer that signifies the time since epoch. Not passing a value defaults to the function taking the current time.
	 * \return A string with the format "%Y-%m-%d %X".
	 */
	static std::string	getCurrentTime		(std::int64_t timestamp = 0);

	/**
	 * @brief Gets the current timestamp.
	 * 
	 * \return a 64-bit integer that signifies the time since epoch.
	 */
	static int64_t		getCurrentTimeStamp	();

	/**
	 * @brief Parses the CRUD operation and turns it into an informative, human readable format.
	 * 
	 * \param op the CRUD operation being executed.
	 * \param user username of the user executing the CRUD operation
	 * \param equipment name of the equipment involved. For CrudOperation::View, this should be the name of the Main Category of equipment.
	 * \param equipmentID id of the equipment
	 * \param timestamp timestamp of when this operation is getting executed
	 * \param parsedTime the human readable version of timestamp
	 * \return empty string if parsing failed, or the actual parsed string if successful
	 */
	static std::string	parseOperation		(CrudOperation& op, 
											 std::string& user, 
											 std::string& equipment, 
											 int equipmentID, 
											 int64_t timestamp, 
											 std::string& parsedTime);
};

