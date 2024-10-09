#include "jwt-cpp/jwt.h"
#include "jwt_handler.h"
#include <chrono>

const std::string jwt_handler::secretKey = ":33333"; // Ideally this'll be stored somewhere secret (malamang)
const std::string jwt_handler::issuer = "lantaka-IMS-RESTAPI";

std::string jwt_handler::create_token(std::string username)
{
    return jwt::create()
        .set_issuer(jwt_handler::issuer)
        .set_type("JWT")
        .set_issued_at(std::chrono::system_clock::now())
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(3)) // JWT Token Expires at 3 hours after creation
        .set_payload_claim("username", jwt::claim(std::string(username)))
        .sign(jwt::algorithm::hs256{ secretKey });
}

TokenResponse jwt_handler::validate_token(std::string token)
{
	try{
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .with_type("JWT")
            .with_issuer(issuer)
            .allow_algorithm(jwt::algorithm::hs256{ secretKey });

        verifier.verify(decoded);

        return TokenResponse(
            Token::Valid, 
            decoded.get_payload_claim("username").as_string()
        );
	} catch (const jwt::token_verification_exception&) {
        return TokenResponse(Token::Expired);
	} catch (const std::invalid_argument&) {
        return TokenResponse(Token::Malformed);
    } catch (...) {
        return TokenResponse(Token::Unhandled);
    }
}
