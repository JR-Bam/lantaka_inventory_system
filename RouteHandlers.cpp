#include "RouteHandlers.h"
#include "jwt_handler.h"

static const std::string dummyUsername = "admin";
static const std::string dummyPassword = "password";

void RouteHandlers::handle_success_authSess(Response& res, std::string& username)
{
    json response_json = {
        {"status", "success"},
        {"username", username}
    };
    res.set_content(response_json.dump(), "application/json");
    res.status = 200; // HTTP 200 OK
}

void RouteHandlers::handle_success_verifyAcc(Response& res, std::string& username)
{
    json response_json = {
        {"status", "success"},
        {"token", jwt_handler::create_token(username)}
    };
    res.set_content(response_json.dump(), "application/json");
    res.status = 200; // HTTP 200 OK
}

void RouteHandlers::handle_unauthorized(Response& res)
{
    json response_json = {
        {"status", "error"},
        {"errorType", "UNAUTHORIZED_LOGIN"}
    };
    res.set_content(response_json.dump(), "application/json");
    res.status = 401; // HTTP 401 Unauthorized
}

void RouteHandlers::handle_bad_request(Response& res)
{
    json response_json = {
        {"status", "error"},
        {"errorType", "BAD_REQUEST"}
    };

    res.set_content(response_json.dump(), "application/json");
    res.status = 400;  // HTTP 400 Bad Request
}

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
            handle_unauthorized(res);
        }
    } catch (const std::exception&) {
        handle_bad_request(res);
    }
}

void RouteHandlers::authSession(const Request& req, Response& res)
{
    json response_json;
    if (!req.has_header("Authorization")) {
        handle_bad_request(res);
        return;
    }
        
    try {
        const std::string token = req.get_header_value("Authorization").substr(7);
        TokenResponse tokenResponse = jwt_handler::validate_token(token);

        if (tokenResponse.valid) {
            handle_success_authSess(res, tokenResponse.username);
        } else {
            handle_unauthorized(res);
        }
    } catch (const std::exception&) {
        handle_bad_request(res);
    }
    
}
