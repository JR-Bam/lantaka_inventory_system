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

void RouteHandlers::addEquipment(const Request& req, Response& res) { //working na yehey
    json req_json;
    try {
        req_json = json::parse(req.body);

        // Validate required fields

        if (!req_json.contains("I_Product") || !req_json.contains("I_Quantity") ||
            !req_json.contains("I_SN") || !req_json.contains("UNIT_ID") ||
            !req_json.contains("I_Location") || !req_json.contains("E_Storage")) {
            handle_error_api(res, "Missing required fields", StatusCode::BadRequest_400);
            return;
        }

        // Connect to the database
        sql::Connection* con = MySQLManager::connectDB();
        if (!con) {
            handle_error_api(res, "Database connection failed", StatusCode::InternalServerError_500);
            return;
        }

        // Prepare the SQL statement
        sql::PreparedStatement* pstmt = con->prepareStatement("INSERT INTO inventory (I_Product, I_SN, I_Quantity, UNIT_ID, I_Location, E_Storage) VALUES (?, ?, ?, ?, ?, ?)");


        pstmt->setString(1, req_json["I_Product"].get<std::string>());
        pstmt->setString(2, req_json["I_SN"].get<std::string>());
        pstmt->setInt(3, req_json["I_Quantity"].get<int>());
        pstmt->setInt(4, req_json["UNIT_ID"].get<int>()); 
        pstmt->setString(5, req_json["I_Location"].get<std::string>());
        pstmt->setString(6, req_json["E_Storage"].get<std::string>());


        // Execute the statement
        pstmt->executeUpdate();
        handle_success_api(res, "Equipment added successfully");

        // Clean up
        delete pstmt;
        delete con;
    }
    catch (const std::exception& e) {
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
        for (json& field : updateFields){ // Gets every key-value pair from the array within the updateFields
            for (auto it = field.begin(); it != field.end(); it++) {
                params.push_back(std::make_pair(it.key(), it.value().dump()));
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
