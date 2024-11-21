#pragma once

#include "httplib.h"
#include "json.hpp"
#include <string>


using namespace httplib;
using json = nlohmann::json;

class RouteHandlers
{
private:
	static std::string getUsername(const Request& req);
	static std::string getCategory(const Request& req);
	static void handle_success_authSess(Response& res, std::string& username);
	static void handle_success_verifyAcc(Response& res, std::string& username);
	
	static void handle_success_api(Response& res, const std::string& message);
	static void handle_error_api(Response& res, const std::string& message, int status_code);
	static void handle_success_view(Response& res, const std::string& message, json jsonArray);

public:

	static void verifyAccount(const Request& req, Response& res); // Reworked
	static void authSession(const Request& req, Response& res);

	static void signUp(const Request& req, Response& res);
	static void addEquipment(const Request& req, Response& res);
	static void viewEquipment(const Request& req, Response& res);
	static void editEquipment(const Request& req, Response& res); // Reworked
	static void removeEquipment(const Request& req, Response& res);
};




