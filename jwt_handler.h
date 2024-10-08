#pragma once
#include <string>
class jwt_handler
{
private:
	static const std::string secretKey;
	static const std::string issuer;
public:
	static std::string create_token(std::string);
	static bool isValidToken(std::string);
	static std::string getUsername(std::string);

};

