/*****************************************************************//**
 * \file   RouteHandlers.h
 * \brief  A list of functions that handle the functionality for the server's endpoints.
 * 
 * \date   November 2024
 *********************************************************************/
#pragma once

#include "httplib.h"
#include "json.hpp"
#include <string>


using namespace httplib;
using json = nlohmann::json;


class RouteHandlers
{
private:
	/**
	 * Extracts the username from the request object.
	 * 
	 * It does this by looking for a "token" param in the request path and extracts the value.
	 * It then uses jwt_handler::validate_token() to check if the JWT token is valid 
	 * 
	 * \param req a const reference to the Request object 
	 * \return an empty string if the "token" param is not found or the token is not valid. Otherwise, the username is returned.
	 */
	static std::string	getUsername				(const Request& req);

	/**
	*This function takes E_Storage and stores whatever the value associated to
	*E_Storage into string storage and returns it
	* 
	*Precondition: req has E_Storage which should have value
	* 
	*@param req It should include the key "E_Storage" in it which holds value representing storage category
	* 
	*@return is a string which either consists of nothing or the value of E_Storage*/
	static std::string	getCategory				(const Request& req);

	static void			handle_success_authSess	(Response& res, 
												 std::string& username);
	static void			handle_success_verifyAcc(Response& res, 
												 std::string& username);
	
	static void			handle_success_api		(Response& res, 
												 const std::string& message);
	static void			handle_error_api		(Response& res, 
												 const std::string& message, 
												 int status_code);
	static void			handle_success_view		(Response& res, 
												 const std::string& message, 
												 json jsonArray);

public:

	/**
	 * @brief POST handler of login attempts from the client.
	 * 
	 * @param req The HTTP request object. Expects a JSON body with the following structure:
	 * ```json
	 * {
	 *	"username": "<string>",
	 *	"password": "<string>"
	 * }
	 * ```
	 * 
	 * @param res The HTTP response object. Returns a 200 OK response on success with a JSON payload containing a JWT session token.
	 * 
	 * 
	 * ## Success Response Body
	 * ```json
	 * {
	 *	"status": "success",
	 *	"token": "<token_string>"
	 * }
	 * ```
	 * 
	 * ## Error Response Body
	 * In case on an error, res will return a response with the corresponding HTTP status and a JSON payload:
	 * ```json
	 * {
	 *	"status": "error"
	 *	"message: "<string>"
	 * }
	 * ```
	 */
	static void verifyAccount	(const Request& req, Response& res);

	/**
	 * @brief GET handler to validate login sessions.
	 * 
	 * @param req The HTTP request object. Expects an authorization header that contains the session token.
	 * ```js 
	 * Authorization: Bearer <token> 
	 * ```
	 * @param res The HTTP response object. Returns a 200 OK response if token is valid with a JSON body containing the username.
	 * 
	 * ## Success Response Body
	 * ```json
	 * {
	 *	"status": "success",
	 *	"username": "<username>"
	 * }
	 * ```
	 * 
	 * ## Error Response Body
	 * In case on an error, res will return a response with the corresponding HTTP status and a JSON payload:
	 * ```json
	 * {
	 *	"status": "error",
	 *	"message: "<string>"
	 * }
	 * ```
	 */
	static void authSession		(const Request& req, Response& res);

	static void signUp			(const Request& req, Response& res);
	static void addEquipment	(const Request& req, Response& res);
	/**
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
	static void viewEquipment	(const Request& req, Response& res);

	/**
	 * @brief PUT handler to edit an existing equipment.
	 * 
	 * @param req The HTTP request object. Expects a JSON body with the following structure:
	 * ```json
	 * {
	 *	  "EquipmentID": 21,
	 *	  "Updates": [
	 *		  {"<Column_Name>": <Value>},
	 *		  {"<Column_Name>": <Value>}
	 *	  ]
	 * }
	 * ```
	 * @note The array for "Updates" is not limited to two elements, it depends on the values the client wishes to edit.
	 * 
	 * @param res The HTTP response object. Returns a 200 OK response if the edit was successful.
	 * 
	 * ## Success and Error Response Body
	 * The response bodies for both cases are almost identical. Both kinds of responses can be differentiated by the HTTP Status Code and/or message.
	 * ```json
	 * {
	 *	"status": "<success/error>",
	 *  "message": "string"
	 * }
	 * ```
	 */
	static void editEquipment	(const Request& req, Response& res);
	/**
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
	static void removeEquipment	(const Request& req, Response& res);
};




