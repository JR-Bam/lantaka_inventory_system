#include "RouteHandlers.h"
#include "jwt_handler.h"

static const std::string dummyUsername = "admin";
static const std::string dummyPassword = "password";

void RouteHandlers::verifyAccount(const Request& req, Response& res)
{
    try {
        json req_json = json::parse(req.body);
        std::string username = req_json.at("username");
        std::string password = req_json.at("password");

        // In a real life context the password should be hashed/encypted before being sent here
        // "But this is already a real life context" I do not care
        if (username == dummyUsername && password == dummyPassword) {
            handle_success_verifyAcc(res, username);
        } else {
            handle_unauthorized(res, "Username or Password is Wrong");
        }
    } catch (const std::exception&) {
        handle_bad_request(res, "Username and/or password were not provided");
    }
}

void RouteHandlers::authSession(const Request& req, Response& res)
{
    json response_json;
    if (!req.has_header("Authorization")) {
        handle_bad_request(res, "No `Authorization` header provided");
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
            handle_bad_request(res, "Token or header format is not correct");
            break;
            
        case Token::Expired:
            handle_unauthorized(res, "Token Expired");
            break;
    }

}

void RouteHandlers::addEquipment(const Request& req, Response& res) {
    json req_json;
    try {
        req_json = json::parse(req.body);

        // Validate required fields
        if (!req_json.contains("ID") || !req_json.contains("Product_Name") ||
            !req_json.contains("Serial_Number") || !req_json.contains("Quantity") ||
            !req_json.contains("Unit") || !req_json.contains("Location") ||
            !req_json.contains("Storage")) {
            handle_error_insert(res, "Missing required fields", StatusCode::BadRequest_400);
            return;
        }

        // Connect to the database
        sql::Connection* con = connectDB();
        if (!con) {
            handle_error_insert(res, "Database connection failed", StatusCode::InternalServerError_500);
            return;
        }


        sql::PreparedStatement* pstmt = con->prepareStatement("INSERT INTO equipment (ID, Product_Name, Serial_Number, Quantity, Unit, Location, Storage) VALUES (?, ?, ?, ?, ?, ?, ?)");


        pstmt->setInt(1, req_json["ID"].get<int>());
        pstmt->setString(2, req_json["Product_Name"].get<std::string>());
        pstmt->setString(3, req_json["Serial_Number"].get<std::string>());
        pstmt->setInt(4, req_json["Quantity"].get<int>());
        pstmt->setString(5, req_json["Unit"].get<std::string>());
        pstmt->setString(6, req_json["Location"].get<std::string>());
        pstmt->setString(7, req_json["Storage"].get<std::string>());

        // Execute the statement
        pstmt->executeUpdate();
        handle_success_insert(res, "Equipment added successfully");

        // Clean up
        delete pstmt;
        delete con;
    }
    catch (const std::exception& e) {
        handle_error_insert(res, std::string("Error occurred: ") + e.what(), StatusCode::InternalServerError_500);
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
        "errorType": ...,
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

void RouteHandlers::handle_unauthorized(Response& res, std::string message)
{
    json response_json = {
        {"status", "error"},
        {"errorType", "UNAUTHORIZED"},
        {"message", message}
    };
    res.set_content(response_json.dump(), "application/json");
    res.status = StatusCode::Unauthorized_401; // HTTP 401 Unauthorized
}

void RouteHandlers::handle_bad_request(Response& res, std::string message)
{
    json response_json = {
        {"status", "error"},
        {"errorType", "BAD_REQUEST"},
        {"message", message}
    };

    res.set_content(response_json.dump(), "application/json");
    res.status = StatusCode::BadRequest_400;  // HTTP 400 Bad Request
}




void RouteHandlers::handle_success_insert(Response& res, const std::string& message) {
    json response_json = {
        {"status", "success"},
        {"message", message}
    };
    res.set_content(response_json.dump(), "application/json");
    res.status = StatusCode::OK_200; // HTTP 200 OK
}

void RouteHandlers::handle_error_insert(Response& res, const std::string& message, int status_code) {
    json response_json = {
        {"status", "error"},
        {"message", message}
    };
    res.set_content(response_json.dump(), "application/json");
    res.status = status_code; // Set the provided status code
}

sql::Connection* RouteHandlers::connectDB() {
    sql::mysql::MySQL_Driver* driver;
    sql::Connection* con;

    try {
        // Set the username, password, and dbname on RouteHandlers.h
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect("mysql://127.0.0.1:3306", SQLConsts::username, SQLConsts::password);
        con->setSchema(SQLConsts::dbName);
    }
    catch (sql::SQLException& e) {
        std::cerr << "Error connecting to the database: " << e.what() << std::endl;
        return nullptr;
    }
    return con;
}
