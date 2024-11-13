#include "MySQLManager.h"
#include <iostream>
#include <bcrypt/BCrypt.hpp>

sql::Connection* MySQLManager::connectDB()
{
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

mysqlx::Schema* MySQLManager::getDatabase(const std::string& name)
{
    try {
        mysqlx::Schema* schema = new mysqlx::Schema(instance().session.getSchema(name));
        return schema;
    }
    catch (mysqlx::Error& e) {
        std::cerr << "Error connecting to the database: " << e.what() << std::endl;
        return nullptr;
    }
}

LoginResult MySQLManager::isValidCredentials(const std::string& username, const std::string& password)
{
    using namespace mysqlx;
    try {
        Schema IMS = instance().session.getSchema(SQLConsts::dbName);
        Table users = IMS.getTable("users");

        RowResult matchedUsers = users
            .select(SQLColumn::USER_UNAME, SQLColumn::USER_PWORD)
            .where("U_Username = :input")
            .bind("input", username)
            .execute();
        
        if (matchedUsers.count() == 0) // If no usernames match
            return LoginResult::BadCredentials;
        
        for (const Row& user : matchedUsers) {
            std::string passwordHash = user[1].get<std::string>();
            if (BCrypt::validatePassword(password, passwordHash)) {
                return LoginResult::Success;
            }
        }

        return LoginResult::BadCredentials;
    }
    catch (mysqlx::Error& e) {
        std::cerr << "MySQLX Error: " << e.what() << std::endl;
        return LoginResult::InternalServerError;
    }
}
