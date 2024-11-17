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

MySQLResult MySQLManager::signUp(const std::string& username, const std::string& password) {
    try {
        std::string hashedPass = BCrypt::generateHash(password);

        mysqlx::Table users = instance().session.getSchema(SQLConsts::dbName).getTable("users");
        users.insert(SQLColumn::USER_UNAME, SQLColumn::USER_PWORD)
            .values(username, hashedPass).execute();
        return MySQLResult::Success;
    }
    catch (const mysqlx::Error& err) {
        std::cerr << "Error adding user: " << err.what() << std::endl;
        return MySQLResult::InternalServerError;
    }
}


MySQLResult MySQLManager::updateEquipment(const int id, const std::vector<std::pair<std::string, std::string>>& params)
{
    using namespace mysqlx;
    try {
        Schema IMS = instance().session.getSchema(SQLConsts::dbName);
        Table inventory = IMS.getTable("inventory");

        TableUpdate update = inventory.update();        // Initialize update query statement
        for (const auto& pair : params) {               // sets the parameters
            update.set(pair.first, pair.second);
        }
        Result queryResult = update
            .where("I_ID = :id")                        // sets the id of the equipment to be editted
            .bind("id", id)                             // bind values
            .execute();                                 // finally execute update query statement

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

MySQLResult MySQLManager::addEquipment(const std::string& product, const std::string& serial_num, int quantity, int unit_id, const std::string& location, const std::string& storage) {
    try {
        mysqlx::Table inventory = instance().session.getSchema(SQLConsts::dbName).getTable("inventory");
        inventory.insert(SQLColumn::EQ_NAME, SQLColumn::EQ_SERIAL_NUM, SQLColumn::EQ_QUANTITY, SQLColumn::EQ_UNIT_ID, SQLColumn::EQ_LOCATION, SQLColumn::EQ_STORAGE)
            .values(product, serial_num, quantity, unit_id, location, storage).execute();

        return MySQLResult::Success;
    }
    catch (const mysqlx::Error& err) {
        std::cerr << "Error adding equipment: " << err.what() << std::endl;
        return MySQLResult::InternalServerError;
    }
}
mysqlx::RowResult MySQLManager::viewEquipment() {
    try {
        mysqlx::Schema db = instance().session.getSchema("lantaka_ims");
        mysqlx::Table inventory = db.getTable("inventory");
        return inventory.select("*").execute();
    }
    catch (const mysqlx::Error& err) {
        std::cerr << "Error viewing equipment: " << err.what() << std::endl;
        return mysqlx::RowResult();
    }
}
std::string MySQLManager::queryEquipment(const int& unit_id) {
    try {
        mysqlx::Schema db = instance().session.getSchema("lantaka_ims");
        mysqlx::Table unitTable = db.getTable("unit");

        mysqlx::RowResult unit_query_result = unitTable
            .select("UN_Name")
            .where("UN_ID = :unit_id")
            .bind("unit_id", unit_id)
            .execute();

        if (unit_query_result.count() > 0) {
            mysqlx::Row row = unit_query_result.fetchOne();
            std::string unit_name = row[0].get<std::string>();
            return unit_name;
        }
        else {
            return "Unit not found";
        }
    }
    catch (const mysqlx::Error& err) {
        return "error";
    }
}