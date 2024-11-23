/*****************************************************************//**
 * \file   jwt_handler.h
 * \brief  Creates and Validates JSON Web Tokens (JWTs)
 * 
 * \date   November 2024
 *********************************************************************/
#pragma once
#include <string>


namespace Token {
	/**
	 * @brief the status of the token.
	 */
	enum Status {
		Valid, /// Token is valid
		Expired, /// Token is past its lifetime of 3 hours
		Malformed, /// Token is not in the right format 
		Unhandled /// Condition has not been addressed
	};
}

/**
 * \brief A struct that wraps around the username that's returned from jwt_handler::validate_token with the status of the token. 
 */
struct TokenResponse {
	Token::Status status;
	std::string username;

	/**
	 * A constructor for token response. Leaving out the username when using the constructor makes it an empty string,
	 * which should be used for non-valid Token::Status.
	 * 
	 * \param status
	 * \param username
	 */
	TokenResponse(Token::Status status, std::string username = "") 
		: status{ status }, username{ username } {}
};


class jwt_handler
{
private:
	static const std::string secretKey; /*!< the key used for creating and validating tokens */
	static const std::string issuer; /*!< the name of the issuer */
public:
	/**
	 * Creates a token given a username. A token has a lifetime of 3 hours
	 * \param username
	 * 
	 * \return a string that contains the token.
	 */
	static std::string		create_token	(std::string& username);
	/**
	 * Validates a token by checking if it's expired, or malformed.
	 * \param token the string containing the token
	 * 
	 * \return a TokenResponse containing the username if the token is valid, or an empty string otherwise. 
	 * Either way a Token::Status will be returned to specify the result.
	 */
	static TokenResponse	validate_token	(std::string& token);

};

