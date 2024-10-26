#pragma once

#include "httplib.h"
#include "json.hpp"
#include <mysql/jdbc.h>
#include <string>


using namespace httplib;
using json = nlohmann::json;

namespace SQLConsts {
	const std::string username = "root";
	const std::string password = "";
	const std::string dbName = "db";
}

namespace SQLColumn {
	const std::string EQ_ID = "I_ID";
	const std::string EQ_NAME = "I_Product";
	const std::string EQ_SERIAL_NUM = "I_SN";
	const std::string EQ_QUANTITY = "I_Quantity";
	const std::string EQ_UNIT_ID = "UNIT_ID";
	const std::string EQ_LOCATION = "I_Location";
	const std::string EQ_STORAGE = "E_Storage";
}

class RouteHandlers
{
private:
	static void handle_success_authSess(Response& res, std::string& username);
	static void handle_success_verifyAcc(Response& res, std::string& username);
	static void handle_unauthorized(Response& res, std::string message);
	static void handle_bad_request(Response& res, std::string message);
	
	static void handle_success_insert(Response& res, const std::string& message);
	static void handle_error_sql(Response& res, const std::string& message, int status_code);
	static void handle_success_view(Response& res, const std::string& message, int status_code, json jsonArray);
	static sql::Connection* connectDB();

public:
	static void verifyAccount(const Request& req, Response& res);
	static void authSession(const Request& req, Response& res);

	static void addEquipment(const Request& req, Response& res);
	static void viewEquipment(const Request& req, Response& res);
};




