#pragma once
#include <string>

namespace Token {
	enum Status {
		Valid,
		Expired,
		Malformed,
		Unhandled
	};
}


struct TokenResponse {
	Token::Status status;
	std::string username;

	TokenResponse(Token::Status status, std::string username = "") 
		: status{ status }, username{ username } {}
};

class jwt_handler
{
private:
	static const std::string secretKey;
	static const std::string issuer;
public:
	static std::string		create_token	(std::string);
	static TokenResponse	validate_token	(std::string);

};

