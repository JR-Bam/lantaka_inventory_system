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

MySQLResult MySQLManager::validateCredentials(const std::string& username, const std::string& password)
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
            return MySQLResult::BadCredentials;
        
        for (const Row& user : matchedUsers) {
            std::string passwordHash = user[1].get<std::string>();
            if (BCrypt::validatePassword(password, passwordHash)) {
                return MySQLResult::Success;
            }
        }

        return MySQLResult::BadCredentials;
    }
    catch (mysqlx::Error& e) {
        std::cerr << "MySQLX Error: " << e.what() << std::endl;
        return MySQLResult::InternalServerError;
    }
}

MySQLResult MySQLManager::updateEquipment(const int id, const std::vector<std::pair<std::string, std::string>>& params)
{
    using namespace mysqlx;
    try {
        Schema IMS = instance().session.getSchema(SQLConsts::dbName);
        Table inventory = IMS.getTable("inventory");

        TableUpdate update = inventory.update();        // Not yet finished
        for (const auto& pair : params) {               // sets the parameters
            update.set(pair.first, pair.second);
        }
        Result queryResult = update
            .where("I_ID = :id")                        // sets the id of the equipment to be editted
            .bind("id", id)
            .execute();

        if (queryResult.getAffectedItemsCount() > 0)
            return MySQLResult::Success;
        else
            return MySQLResult::NotFound;
    }
    catch (mysqlx::Error& e) {
        std::cerr << "MySQLX Error: " << e.what() << std::endl;
        return MySQLResult::InternalServerError;
    }
}
