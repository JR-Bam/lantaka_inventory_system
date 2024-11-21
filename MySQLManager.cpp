#include "MySQLManager.h"
#include "Logs.h"
#include <iostream>
#include <bcrypt/BCrypt.hpp>

std::string MySQLManager::getEquipmentUsername(int id)
{
    mysqlx::Table inventory = instance().session.getSchema(SQLConsts::dbName).getTable("inventory");
    return inventory.select(SQLColumn::EQ_NAME)
        .where("I_ID = :inv_id")
        .bind("inv_id", id)
        .execute()
        .fetchOne()[0].get<std::string>();
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
        Logs::logLine(Logs::Type::Error, Logs::Read, "", "", -1, e.what());
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
        Logs::logLine(Logs::Type::Error, Logs::Read, "", "", -1, err.what());
        return MySQLResult::InternalServerError;
    }
}


MySQLResult MySQLManager::updateEquipment(const int id, const std::vector<std::pair<std::string, std::string>>& params, const std::string& username)
{
    using namespace mysqlx;
    try {
        Schema IMS = instance().session.getSchema(SQLConsts::dbName);
        Table inventory = IMS.getTable("inventory");

        std::string equipmentName = getEquipmentUsername(id);

        TableUpdate update = inventory.update();        // Initialize update query statement
        for (const auto& pair : params) {               // sets the parameters
            update.set(pair.first, pair.second);
        }
        Result queryResult = update
            .where("I_ID = :id")                        // sets the id of the equipment to be editted
            .bind("id", id)                             // bind values
            .execute();                                 // finally execute update query statement

        if (queryResult.getAffectedItemsCount() > 0)
        {
            Logs::logLine(Logs::Type::Operation, Logs::CrudOperation::Update, username, equipmentName, id);
            return MySQLResult::Success;
        }
        else
        {
            Logs::logLine(Logs::Type::Error, Logs::Read, "", "", -1, "Equipment ID " + std::to_string(id) + " not found when updating");
            return MySQLResult::NotFound;
        }
    }
    catch (mysqlx::Error& e) {
        Logs::logLine(Logs::Type::Error, Logs::Read, "", "", -1, e.what());
        return MySQLResult::InternalServerError;
    }


}

MySQLResult MySQLManager::addEquipment(const std::string& product, const std::string& serial_num, int quantity, int unit_id, const std::string& location, const std::string& storage, const std::string& username) {
    try {
        mysqlx::Table inventory = instance().session.getSchema(SQLConsts::dbName).getTable("inventory");
        mysqlx::Result result = inventory.insert(SQLColumn::EQ_NAME, SQLColumn::EQ_SERIAL_NUM, SQLColumn::EQ_QUANTITY, SQLColumn::EQ_UNIT_ID, SQLColumn::EQ_LOCATION, SQLColumn::EQ_STORAGE)
            .values(product, serial_num, quantity, unit_id, location, storage).execute();

        Logs::logLine(Logs::Type::Operation, Logs::CrudOperation::Create, username, product, result.getAutoIncrementValue());

        return MySQLResult::Success;
    }
    catch (const mysqlx::Error& err) {
        Logs::logLine(Logs::Type::Error, Logs::Read, "", "", -1, err.what());
        return MySQLResult::InternalServerError;
    }
}
/*This function connects to the mysql database and takes the table inventory from the schema lantaka_ims
and returns all inventory that has E_Storage of storage
Preconditions: username should be valid and storage has value
@param username is used for logging the action
@param storage to determine which equipments/items to show
@return all inventory items/equipments in specific storage*/
mysqlx::RowResult MySQLManager::viewEquipment(const std::string& username,const std::string& storage) {
    try {
        mysqlx::Schema db = instance().session.getSchema("lantaka_ims");
        mysqlx::Table inventory = db.getTable("inventory");

        Logs::logLine(Logs::Type::Operation, Logs::CrudOperation::Read, username, "Housekeeping");

        return inventory.select("*").where("E_Storage = '" + storage + "'").execute();
    }
    catch (const mysqlx::Error& err) {
        Logs::logLine(Logs::Type::Error, Logs::Read, "", "", -1, err.what());
        return mysqlx::RowResult();
    }
}
/*This function connects to the mysql database and takes the table unit from the schema lantaka_ims
and returns the name of the unit associated with the unit_id in the param
Precondition: unit_id corresponds to a equipment and has value
@param unit_id is the primary key or id of the unit associated with the equipment in this instance
@return the name of the unit*/
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
        Logs::logLine(Logs::Type::Error, Logs::Read, "", "", -1, err.what());
        return std::string();
    }
}
/*This function connects to the mysql database and takes the table inventory from schema lantaka_ims
gets the equipment name by running inv_id in param through getEquipmentUsername() function then deletes it
Precondition: inv_id has value and username is valid
@param inv_id is the id of the equipment to delete
@param username is for logging purposes
@return either a success or error*/
MySQLResult MySQLManager::deleteEquipment(const int& inv_id, const std::string& username) {
    try {
        mysqlx::Schema IMS = instance().session.getSchema(SQLConsts::dbName);
        mysqlx::Table inventory = instance().session.getSchema(SQLConsts::dbName).getTable("inventory");

        std::string equipmentName = getEquipmentUsername(inv_id);

        inventory.remove()
            .where("I_ID = :inv_id")
            .bind("inv_id", inv_id)
            .execute();

        Logs::logLine(Logs::Type::Operation, Logs::CrudOperation::Delete, username, equipmentName, inv_id);

        return MySQLResult::Success;
    }
    catch (const mysqlx::Error& err) {
        Logs::logLine(Logs::Type::Error, Logs::Read, "", "", -1, err.what());
        return MySQLResult::InternalServerError;
    }
}