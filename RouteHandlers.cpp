#include "RouteHandlers.h"
#include "jwt_handler.h"
#include "MySQLManager.h"
#include <vector>
#include <sstream>

void RouteHandlers::verifyAccount(const Request& req, Response& res)
{
    try {
        json req_json = json::parse(req.body);
        std::string username = req_json.at("username");
        std::string password = req_json.at("password");

        switch (MySQLManager::validateCredentials(username, password)) {
            case MySQLResult::Success:
                handle_success_verifyAcc(res, username);
                break;
            case MySQLResult::BadCredentials:
                handle_error_api(res, "Username or Password is Wrong", StatusCode::Unauthorized_401);
                break;
            case MySQLResult::InternalServerError:
                handle_error_api(res, "Username and/or password were not provided", StatusCode::InternalServerError_500);
                break;
        }
    } catch (const std::exception&) {
        handle_error_api(res, "Username and/or password were not provided", StatusCode::BadRequest_400);
    }
}

void RouteHandlers::authSession(const Request& req, Response& res)
{
    json response_json;
    if (!req.has_header("Authorization")) {
        handle_error_api(res, "No `Authorization` header provided", StatusCode::BadRequest_400);
        return;
    }
    
    // We expect the header would look like this:
    // Authorization: Bearer <token>
    // So in extracting the token we just need to remove "Bearer " from the header value
    std::string token = req.get_header_value("Authorization").substr(7);
    TokenResponse tokenResponse = jwt_handler::validate_token(token);

    switch (tokenResponse.status) {
        case Token::Valid:
            handle_success_authSess(res, tokenResponse.username);
            break;

        case Token::Malformed: case Token::Unhandled:
            handle_error_api(res, "Token or header format is not correct", StatusCode::BadRequest_400);
            break;
            
        case Token::Expired:
            handle_error_api(res, "Token Expired", StatusCode::Unauthorized_401);
            break;
    }

}

void RouteHandlers::logs(const Request& req, Response& res)
{
    try {
        std::string username = getUsername(req);
        if (username.empty()) {
            handle_error_api(res, "Session expired", StatusCode::Unauthorized_401);
            return;
        }

        std::string logHtmlContent = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Log Viewer</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
</head>
<body>
    <div class="container mt-5">
        <h1 class="text-center mb-4">Log Viewer</h1>
        <div class="text-center mb-4">
            <button class="btn btn-primary" onclick="window.location.href = '/home';">Back to Inventory</button>
            <button class="btn btn-secondary" onclick="scrollToBottom();">Scroll to Bottom</button>
        </div>
        <div class="table-responsive" id="logContainer" style="max-height: 500px; overflow-y: auto;">
            <table class="table table-striped table-hover table-bordered">
                <thead class="table-primary">
                    <tr></tr>
                </thead>
                <tbody id="logTableBody">
                    <!-- Log lines will go here -->
                </tbody>
            </table>
        </div>
    </div>
    <script>
        function scrollToBottom() {
            const logContainer = document.getElementById('logContainer');
            logContainer.scrollTop = logContainer.scrollHeight;
        }
    </script>
</body>
</html>)";

        std::string toFind = "<!-- Log lines will go here -->";
        std::stringstream rows;

        for (const auto row : MySQLManager::getLoglines()) {
            rows << "<tr><td>" << (row[0].isNull() ? "" : row[0].get<std::string>()) << "</td></tr>";
        }

        std::string toReplace = rows.str();
        size_t pos = logHtmlContent.find(toFind);
        if (pos != std::string::npos) {
            logHtmlContent.replace(pos, toFind.length(), toReplace);
            res.set_content(logHtmlContent, "text/html");
        }
        else {
            handle_error_api(res, "Failed to parse html", StatusCode::InternalServerError_500);
        }
    }
    catch (const std::exception& e)
    {
        handle_error_api(res, std::string("Error occurred: ") + e.what(), StatusCode::InternalServerError_500);
    }
}

void RouteHandlers::newPassword(const Request& req, Response& res) {
    try {
        json req_json = json::parse(req.body);

        if (!req_json.contains("username") || !req_json.contains("old_password") || !req_json.contains("new_password")) {
            handle_error_api(res, "Missing required fields", StatusCode::BadRequest_400);
            return;
        }

        std::string username = req_json["username"];
        std::string old_password = req_json["old_password"];
        std::string new_password = req_json["new_password"];

        if (old_password == new_password) {
            handle_success_api(res, "Old password same as new password.");
            return;
        }

        switch (MySQLManager::updatePassword(username, old_password, new_password)) {
        case MySQLResult::Success:
            handle_success_api(res, "User signed up successfully.");
            break;
        case MySQLResult::NotFound:
            handle_error_api(res, "User not found.", StatusCode::NotFound_404);
            break;
        case MySQLResult::BadCredentials:
            handle_error_api(res, "Wrong Password.", StatusCode::Unauthorized_401);
            break;
        case MySQLResult::InternalServerError:
            handle_error_api(res, "Internal server error occurred.", StatusCode::InternalServerError_500);
            break;
        }
    }
    catch (const std::exception& e) {
        handle_error_api(res, std::string("Error occurred: ") + e.what(), StatusCode::InternalServerError_500);
    }
}
/**
* This function handles the addition of new equipment to the inventory.
* It validates the request, extracts required fields, and then calls
* MySQLManager::addEquipment to insert the data into the database.
*
* Precondition: req must be valid and contain required parameters like
* I_Product, I_SN, I_Quantity, UNIT_ID, I_Location, and E_Storage.
*
* @param req is the user's request to add new equipment.
* @param res is the response of the system to the user.
*
* @return if username is empty or invalid, returns a 401 Unauthorized status.
* If required fields are missing, returns a 400 Bad Request status.
* If the addition is successful, returns a success message.
* If an internal server error occurs, returns a 500 Internal Server Error status.
*/
void RouteHandlers::addEquipment(const Request& req, Response& res) {
    try
    {
        std::string token = req.matches[1].str();
        std::string username = getUsername(req);
        if (username.empty()) {
            handle_error_api(res, "Session expired", StatusCode::Unauthorized_401);
            return;
        }

        json req_json = json::parse(req.body);

        // Validate required fields
        if (!req_json.contains("I_Product") || !req_json.contains("I_SN") ||
            !req_json.contains("I_Quantity") || !req_json.contains("UNIT_ID") ||
            !req_json.contains("I_Location") || !req_json.contains("Storage_Category") ||
            !req_json.contains("Storage_Subcategory") || !req_json.contains("Status") ||
            !req_json.contains("Remarks") || !req_json.contains("Date_Received") || 
            !req_json.contains("Date_Released") || !req_json.contains("Released_To"))
        {
            handle_error_api(res, "Missing required fields", StatusCode::BadRequest_400);
            return;
        }

        // Extract fields from JSON
        std::string product = req_json["I_Product"];
        std::string serial_num = req_json["I_SN"];
        double quantity = req_json["I_Quantity"];
        int unit_id = req_json["UNIT_ID"];
        std::string location = req_json["I_Location"];
        std::string storage_category = req_json["Storage_Category"];
        std::string storage_sub = req_json["Storage_Subcategory"];
        std::string status = req_json["Status"];
        std::string remarks = req_json["Remarks"];
        std::string date_received = req_json["Date_Received"];
        std::string date_released = req_json["Date_Released"];
        std::string released_to = req_json["Released_To"];

        // Call the MySQLManager's addEquipment method
        switch (MySQLManager::addEquipment(product, serial_num, quantity, unit_id, location, storage_category, storage_sub, status, remarks, date_received, date_released, released_to, username)) {
        case MySQLResult::Success:
            handle_success_api(res, "Equipment added successfully.");
            break;
        case MySQLResult::InternalServerError:
            handle_error_api(res, "Internal server error occurred.", StatusCode::InternalServerError_500);
            break;
        }
    }
    catch (const std::exception& e)
    {
        handle_error_api(res, std::string("Error occurred: ") + e.what(), StatusCode::InternalServerError_500);
    }
}


/*this function takes the data it gets from the MySQLManager::viewEquipment
and MySQLManager::queryEquipment function and places it all into json array
Precondition: req must be valid and contains required parameters like E_Storage and session information
@param req is the user's request to view the inventory
@param res is the response of the system to the user
@return if username is empty or invalid
*/
void RouteHandlers::viewEquipment(const Request& req, Response& res)
{
    try {
        std::string username = getUsername(req);
        if (username.empty()) {
            handle_error_api(res, "Session expired", StatusCode::Unauthorized_401);
            return;
        }
        std::string storage = getCategory(req);
        auto result = MySQLManager::viewEquipment(username,storage);
        json jsonArray = json::array();

        while (auto row = result.fetchOne()) {
            json jsonRow;

            jsonRow["ID"] = row[0].isNull() ? 0 : row[0].get<int>();
            jsonRow["Product_Name"] = row[1].isNull() ? "" : row[1].get<std::string>();
            jsonRow["Serial_Number"] = row[2].isNull() ? "" : row[2].get<std::string>();
            jsonRow["Quantity"] = row[3].isNull() ? 0 : row[3].get<double>();
            jsonRow["Location"] = row[5].isNull() ? "" : row[5].get<std::string>();
            jsonRow["Storage_Category"] = row[6].isNull() ? "" : row[6].get<std::string>();
            jsonRow["Storage_Subcategory"] = row[7].isNull() ? "" : row[7].get<std::string>();
            jsonRow["Status"] = row[8].isNull() ? "" : row[8].get<std::string>();
            jsonRow["Remarks"] = row[9].isNull() ? "" : row[9].get<std::string>();
            jsonRow["Date_Received"] = row[10].isNull() ? "" : row[10].get<std::string>();
            jsonRow["Date_Released"] = row[11].isNull() ? "" : row[11].get<std::string>();
            jsonRow["Released_To"] = row[12].isNull() ? "" : row[12].get<std::string>();
            
            jsonRow["Unit"]["id"] = row[4].isNull() ? 0 : row[4].get<int>();
            int unit_id = jsonRow["Unit"]["id"];

            if (unit_id != 0) {
                std::string unit_name = MySQLManager::queryEquipment(unit_id);
                jsonRow["Unit"]["name"] = unit_name.empty() ? "" : unit_name;
            }
            else {
                jsonRow["Unit"]["name"] = "";
            }

            jsonArray.push_back(jsonRow);
        }

        res.set_content(jsonArray.dump(1), "application/json");
        handle_success_view(res, "success", jsonArray);
    }
    catch (const std::exception& e) {
        handle_error_api(res, std::string("query execution error: ") + e.what(), 500);
    }
}


void RouteHandlers::editEquipment(const Request& req, Response& res)
{
    try
    {
        std::string username = getUsername(req);
        if (username.empty()) { 
            handle_error_api(res, "Session expired", StatusCode::Unauthorized_401);
            return;
        }

        json req_json = json::parse(req.body);

        if (!req_json.contains("EquipmentID") || !req_json.contains("Updates")) {
            handle_error_api(res, "Missing required fields", StatusCode::BadRequest_400);
            return;
        }

        int equipmentID = req_json["EquipmentID"];
        json updateFields = req_json["Updates"];

        std::vector<std::pair<std::string, std::string>> params;
        for (json& field : updateFields){                               // Gets every key-value pair from the array within the updateFields
            for (auto it = field.begin(); it != field.end(); it++) {
                std::string value = it.value().dump();
                if (value.front() == '\"' && value.back() == '\"') {    // Erases the quotation marks brought forth by funky json parsing
                    value.erase(0, 1);
                    value.pop_back();
                }
                params.push_back(std::make_pair(it.key(), value));
            }
        }

        switch (MySQLManager::updateEquipment(equipmentID, params, username)) {
            case MySQLResult::Success:
                handle_success_api(res, "Equipment editted successfully.");
                break;
            case MySQLResult::NotFound:
                handle_error_api(res, "Equipment with specified ID not found", StatusCode::NotFound_404);
                break;
            case MySQLResult::InternalServerError:
                handle_error_api(res, "Internal server error occured.", StatusCode::InternalServerError_500);
                break;
        }

    }
    catch (const std::exception& e)
    {
        handle_error_api(res, std::string("Error occurred: ") + e.what(), StatusCode::InternalServerError_500);
    }
}


void RouteHandlers::removeEquipment(const Request& req, Response& res)
{
    
    try {
        std::string username = getUsername(req);
        if (username.empty()) {
            handle_error_api(res, "Session expired", StatusCode::Unauthorized_401);
            return;
        }
        int equipmentID = std::stoi(req.matches[1].str());

        switch (MySQLManager::deleteEquipment(equipmentID, username)) {
        case MySQLResult::Success:
            handle_success_api(res, "Equipment deleted successfully.");
            break;
        case MySQLResult::InternalServerError:
            handle_error_api(res, "Internal server error occurred.", StatusCode::InternalServerError_500);
            break;
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        handle_error_api(res, std::string("Error occurred: ") + e.what(), StatusCode::BadRequest_400);
    }
}


/*
    Right now, this REST API has its JSON responses structured in this way:

    Success Response
    {
        "status": "success",
        ...
    }
    
    Error Response
    {
        "status": "error",
        "message": ...
    }

    so when the frontend gets a response, either they can check the HTTP status code,
    or they can just check the status value in the JSON response to see if it's a
    success or an error, if it's an error they can check the type of error by
    looking at errorType.
*/

std::string RouteHandlers::getUsername(const Request& req)
{
    auto token_it = req.params.find("token"); 
    if (token_it == req.params.end()) { 
        return std::string();
    }
    
    std::string token = token_it->second; // Extract the token value
    const TokenResponse result = jwt_handler::validate_token(token);
    if (result.status == Token::Valid) {
        return result.username;
    }
    else {
        return std::string();
    }
}

std::string RouteHandlers::getCategory(const Request& req)
{
    auto storage_it = req.params.find("E_Storage");
    if (storage_it == req.params.end()) {
        return std::string();
    }
    std::string storage = storage_it->second;
    return storage;
}

void RouteHandlers::handle_success_authSess(Response& res, std::string& username)
{
    json response_json = {
        {"status", "success"},
        {"username", username}
    };
    res.set_content(response_json.dump(), "application/json");
    res.status = StatusCode::OK_200; // HTTP 200 OK
}

void RouteHandlers::handle_success_verifyAcc(Response& res, std::string& username)
{
    json response_json = {
        {"status", "success"},
        {"token", jwt_handler::create_token(username)}
    };
    res.set_content(response_json.dump(), "application/json");
    res.status = StatusCode::OK_200; // HTTP 200 OK
}

void RouteHandlers::handle_success_view(Response& res, const std::string& message, json jsonArray)
{
    json response_json = {
        {"status", "success"},
        {"message", message},
        {"equipment", jsonArray}
    };
    res.set_content(response_json.dump(), "application/json");
    res.status = StatusCode::OK_200; // HTTP 200 OK
}





void RouteHandlers::handle_success_api(Response& res, const std::string& message) {
    json response_json = {
        {"status", "success"},
        {"message", message}
    };
    res.set_content(response_json.dump(), "application/json");
    res.status = StatusCode::OK_200; // HTTP 200 OK
}

void RouteHandlers::handle_error_api(Response& res, const std::string& message, int status_code) {
    json response_json = {
        {"status", "error"},
        {"message", message}
    };
    res.set_content(response_json.dump(), "application/json");
    res.status = status_code; // Set the provided status code
}
