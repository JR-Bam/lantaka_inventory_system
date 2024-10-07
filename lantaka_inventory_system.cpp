// lantaka_inventory_system.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "json.hpp"
#include "httplib.h"

using namespace httplib;
using json = nlohmann::json;

const std::string dummyUsername = "admin";
const std::string dummyPassword = "password";

void verifyAccount(const Request& req, Response& res) {
    try{
        json req_json = json::parse(req.body);
        std::string username = req_json.at("username");
        std::string password = req_json.at("password");

        json res_body;

        // In a real life context the password should be hashed/encypted before being sent here
        // "But this is already a real life context" I do not care
        if (username == dummyUsername && password == dummyPassword) {
            res_body["status"] = "success";
            res.status = 200; // HTTP 200 OK
        } else {
            res_body["status"] = "error";
            res_body["errorType"] = "UNAUTHORIZED_LOGIN";
            res.status = 401; // HTTP 401 Unauthorized
        }

        res.set_content(res_body.dump(), "application/json");

    } catch (const std::exception&){
        json response_json;
        response_json["status"] = "error";
        response_json["errorType"] = "BAD_REQUEST";

        res.set_content(response_json.dump(), "application/json");
        res.status = 400;  // HTTP 400 Bad Request
    }
}

int main()
{
    Server svr;

    // For HTML Files
    svr.set_mount_point("/", "./public");
    // For the images
    svr.set_mount_point("/images", "./assets");

    // Routes
    svr.Post("/api/verify", &verifyAccount);

    std::cout << "Starting server..." << std::endl;
    svr.listen("localhost", 8080);

    return 0;
}

