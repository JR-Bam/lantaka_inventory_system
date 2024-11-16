#include "RouteHandlers.h"
#include "jwt_handler.h"
#include "MySQLManager.h"
#include <vector>

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
    const std::string token = req.get_header_value("Authorization").substr(7);
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

void RouteHandlers::signUp(const Request& req, Response& res) {
    try {
        json req_json = json::parse(req.body);

        if (!req_json.contains("username") || !req_json.contains("password") || !req_json.contains("confirm_password")) {
            handle_error_api(res, "Missing required fields", StatusCode::BadRequest_400);
            return;
        }

        std::string username = req_json["username"];
        std::string password = req_json["password"];
        std::string confirm_password = req_json["confirm_password"];

        if (password != confirm_password) {
            handle_error_api(res, "Passwords do not match", StatusCode::BadRequest_400);
            return;
        }

        switch (MySQLManager::signUp(username, password)) {
        case MySQLResult::Success:
            handle_success_api(res, "User signed up successfully.");
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

void RouteHandlers::addEquipment(const Request& req, Response& res) {
    try
    {
        json req_json = json::parse(req.body);

        // Validate required fields
        if (!req_json.contains("I_Product") || !req_json.contains("I_SN") ||
            !req_json.contains("I_Quantity") || !req_json.contains("UNIT_ID") ||
            !req_json.contains("I_Location") || !req_json.contains("E_Storage")) {
            handle_error_api(res, "Missing required fields", StatusCode::BadRequest_400);
            return;
        }

        // Extract fields from JSON
        std::string product = req_json["I_Product"];
        std::string serial_num = req_json["I_SN"];
        int quantity = req_json["I_Quantity"];
        int unit_id = req_json["UNIT_ID"];
        std::string location = req_json["I_Location"];
        std::string storage = req_json["E_Storage"];


        // Call the MySQLManager's addEquipment method
        switch (MySQLManager::addEquipment(product, serial_num, quantity, unit_id, location, storage)) {
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


void RouteHandlers::viewEquipment(const Request& req, Response& res)
{
    try {
        sql::Connection* con = MySQLManager::connectDB();
        if (!con) {
            handle_error_api(res, "Database connection failed", 500);
            return;
        }
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT * FROM inventory");
        sql::ResultSet* result(pstmt->executeQuery());

        json jsonArray = json::array();

        while (result->next()) {
            json jsonRow;

            jsonRow["ID"]               = result->getInt(SQLColumn::EQ_ID);
            jsonRow["Product_Name"]     = result->getString(SQLColumn::EQ_NAME);
            jsonRow["Serial_Number"]    = result->getString(SQLColumn::EQ_SERIAL_NUM);
            jsonRow["Quantity"]         = result->getInt(SQLColumn::EQ_QUANTITY);
            jsonRow["Location"]         = result->getString(SQLColumn::EQ_LOCATION);
            jsonRow["Storage"]          = result->getString(SQLColumn::EQ_STORAGE);

            int unit_id = result->getInt(SQLColumn::EQ_UNIT_ID);
            jsonRow["Unit"]["id"]       = unit_id;
            if (unit_id != 0){
                sql::PreparedStatement* unit_query = con->prepareStatement("SELECT * FROM unit WHERE UN_ID = ?");
                unit_query->setInt(1, unit_id);

                sql::ResultSet* unit_query_result = unit_query->executeQuery();
                unit_query_result->next();

                jsonRow["Unit"]["name"] = unit_query_result->getString("UN_Name");

                delete unit_query;
                delete unit_query_result;
            }
            else {
                jsonRow["Unit"]["name"] = nullptr;
            }
            jsonArray.push_back(jsonRow);
        }
        res.set_content(jsonArray.dump(1), "application/json");
        handle_success_view(res, "success", jsonArray);
    }
    catch(const std::exception& e){
        handle_error_api(res,std::string("query execution error: ") + e.what(), 500);
    }
    
}



void RouteHandlers::editEquipment(const Request& req, Response& res)
{
    /*
    * Edit JSON FOrmat
    {
      "EquipmentID": 21,
      "Updates": [
            {"<Column_Name>": <Value>},
            {"<Column_Name>": <Value>}
        ]
    }
    */
    try
    {
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

        switch (MySQLManager::updateEquipment(equipmentID, params)) {
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
        int equipmentID = std::stoi(req.matches[1].str());
        sql::Connection* con = MySQLManager::connectDB();

        if (!con) {
            handle_error_api(res, "Database connection failed", StatusCode::InternalServerError_500);
            return;
        }

        sql::PreparedStatement* pstmt = con->prepareStatement("DELETE FROM inventory WHERE " + SQLColumn::EQ_ID + " = ?");
        pstmt->setInt(1, equipmentID);
        pstmt->executeUpdate();

        delete pstmt;
        delete con;

        handle_success_api(res, "Equipment deleted successfully.");
    } catch (const std::exception& e) {
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
