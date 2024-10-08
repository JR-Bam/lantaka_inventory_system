#pragma once
#include <string>

struct TokenResponse {
	bool valid;
	std::string username;

	TokenResponse(bool valid)
		: valid{ valid }, username{""} {}
	TokenResponse(bool valid, std::string username) 
		: valid{ valid }, username{ username } {}
};

class jwt_handler
{
private:
	static const std::string secretKey;
	static const std::string issuer;
public:
	static std::string create_token(std::string);
	static TokenResponse validate_token(std::string);

};

