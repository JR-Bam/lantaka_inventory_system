#include "RouteHandlers.h"

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

void RouteHandlers::handle_unauthorized(Response& res, std::string message)
{
    json response_json = {
        {"status", "error"},
        {"errorType", "UNAUTHORIZED"},
        {"message", message}
    };
    res.set_content(response_json.dump(), "application/json");
    res.status = 401; // HTTP 401 Unauthorized
}

void RouteHandlers::handle_bad_request(Response& res, std::string message)
{
    json response_json = {
        {"status", "error"},
        {"errorType", "BAD_REQUEST"},
        {"message", message}
    };

    res.set_content(response_json.dump(), "application/json");
    res.status = 400;  // HTTP 400 Bad Request
}
