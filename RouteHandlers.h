#pragma once
#include "httplib.h"
#include "json.hpp"

using namespace httplib;
using json = nlohmann::json;

class RouteHandlers
{
private:
	static void handle_success_authSess(Response& res, std::string& username);
	static void handle_success_verifyAcc(Response& res, std::string& username);
	static void handle_unauthorized(Response& res);
	static void handle_bad_request(Response& res);
public:
	static void verifyAccount(const Request& req, Response& res);
	static void authSession(const Request& req, Response& res);
};

