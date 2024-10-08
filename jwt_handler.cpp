#include "jwt_handler.h"
#include "jwt-cpp/jwt.h"

const std::string jwt_handler::secretKey = "lantaka-IMS-RESTAPI";
const std::string jwt_handler::issuer = ":333333";

std::string jwt_handler::create_token(std::string username)
{
    return jwt::create()
        .set_issuer(jwt_handler::issuer)
        .set_type("JWT")
        .set_payload_claim("username", jwt::claim(std::string(username)))
        .sign(jwt::algorithm::hs256{ secretKey });
}

bool jwt_handler::isValidToken(std::string token)
{
	try{
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .with_type("JWT")
            .with_issuer(issuer)
            .allow_algorithm(jwt::algorithm::hs256{ secretKey });

        verifier.verify(decoded);

        return true;
	} catch (const std::exception&) {
        return false;
	}
}

std::string jwt_handler::getUsername(std::string token)
{
    try {
        return jwt::decode(token).get_payload_claim("username").as_string();
    }
    catch (const std::exception&) {
        return "";
    }
}
