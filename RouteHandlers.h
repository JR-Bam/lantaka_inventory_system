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
	/*
	*This function takes E_Storage and stores whatever the value associated to
	*E_Storage into string storage and returns it
	* 
	*Precondition: req has E_Storage which should have value
	* 
	*@param req It should include the key "E_Storage" in it which holds value representing storage category
	* 
	*@return is a string which either consists of nothing or the value of E_Storage*/
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
	/*
	*this function takes the data it gets from the MySQLManager::viewEquipment
	*and MySQLManager::queryEquipment function and places it all into json array
	* 
	*Precondition: req must be valid and contains required parameters like E_Storage and session information
	* 
	*@param req is the user's request to view the inventory
	*@param res is the response of the system to the user
	* 
	*@return if username is empty or invalid
	*/
	static void viewEquipment(const Request& req, Response& res);
	static void editEquipment(const Request& req, Response& res); // Reworked
	/*
	*This function gets the equipmentID of the equipment/item the user wants to delete
	*and send it to MySQLManager::deleteEquipment to delete
	* 
	*Precondition: req must contain equipment ID
	* 
	*@param req is the equipment the user wants to delete
	*@param res is whether the deletion is a success or an error has occured
	* 
	*@return when username is unavailable or empty 
	*/
	static void removeEquipment(const Request& req, Response& res);
};




