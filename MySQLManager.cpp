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

MySQLResult MySQLManager::logOp(std::string& type, std::string& user, int equipmentID, int64_t timestamp, std::string& parsedTime)
{
    using namespace mysqlx;
    try
    {
        Schema IMS = instance().session.getSchema(SQLConsts::dbName);
        Table users = IMS.getTable("users");
        Table logs = IMS.getTable("management");

        int userID = users
            .select(SQLColumn::USER_ID)
            .where(SQLColumn::USER_UNAME + " = :user")
            .bind("user", user)
            .execute()
            .fetchOne()[0].get<int>();

        logs.insert(SQLColumn::LOGS_TYPE,
                SQLColumn::LOGS_TSTAMP,
                SQLColumn::LOGS_U_ID,
                SQLColumn::LOGS_E_ID,
                SQLColumn::LOGS_P_TIME)
            .values(type,
                timestamp,
                userID,
                equipmentID,
                parsedTime
            ).execute();

        return MySQLResult::Success;
    }
    catch (const mysqlx::Error& err)
    {
        Logs::logLine(Logs::Type::Error, Logs::CrudOperation::Read, "", "", -1, err.what());
        return MySQLResult::InternalServerError;
    }
}

MySQLResult MySQLManager::appendLogline(const std::string& logEntry)
{
    using namespace mysqlx;
    try
    {
        Schema IMS = instance().session.getSchema(SQLConsts::dbName);
        Table loglines = IMS.getTable("loglines");

        loglines.insert("line").values(logEntry).execute();

        return MySQLResult::Success;
    }
    catch (const std::exception& err)
    {
        Logs::logLine(Logs::Type::Error, Logs::CrudOperation::Read, "", "", -1, err.what());
        return MySQLResult::InternalServerError;
    }
    
}

mysqlx::RowResult MySQLManager::getLoglines()
{
    using namespace mysqlx;
    try
    {
        Schema IMS = instance().session.getSchema(SQLConsts::dbName); 
        Table loglines = IMS.getTable("loglines");

        return loglines.select("line").execute();
    }
    catch (const std::exception& err)
    {
        Logs::logLine(Logs::Type::Error, Logs::CrudOperation::Read, "", "", -1, err.what());
        return RowResult();
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
        Logs::logLine(Logs::Type::Error, Logs::CrudOperation::Read, "", "", -1, e.what());
        return MySQLResult::InternalServerError;
    }
}

MySQLResult MySQLManager::updatePassword(const std::string& username, const std::string& old, const std::string& password) {
    try {
        std::string hashedPass = BCrypt::generateHash(password);

        mysqlx::Table users = instance().session.getSchema(SQLConsts::dbName).getTable("users");
        mysqlx::RowResult ids = users.select(SQLColumn::USER_ID, SQLColumn::USER_PWORD)
            .where(SQLColumn::USER_UNAME + " = :u_un")
            .bind("u_un", username).execute();

        mysqlx::Row res = ids.fetchOne();
        if (!res) {
            return MySQLResult::NotFound;
        }

        if (!BCrypt::validatePassword(old, res[1].get<std::string>())) {
            return MySQLResult::BadCredentials;
        }

        users.update()
            .set(SQLColumn::USER_PWORD, hashedPass)
            .where(SQLColumn::USER_ID + " = :u_id")
            .bind("u_id", res[0].get<int>()).execute();
        return MySQLResult::Success;
        
    }
    catch (const mysqlx::Error& err) {
        std::cerr << "Error adding user: " << err.what() << std::endl;
        Logs::logLine(Logs::Type::Error, Logs::CrudOperation::Read, "", "", -1, err.what());
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
            Logs::logLine(Logs::Type::Error, Logs::CrudOperation::Read, "", "", -1, "Equipment ID " + std::to_string(id) + " not found when updating");
            return MySQLResult::NotFound;
        }
    }
    catch (mysqlx::Error& e) {
        Logs::logLine(Logs::Type::Error, Logs::CrudOperation::Read, "", "", -1, e.what());
        return MySQLResult::InternalServerError;
    }


}

MySQLResult MySQLManager::addEquipment(
    const std::string& product, 
    const std::string& serial_num, 
    double quantity, 
    int unit_id, 
    const std::string& location, 
    const std::string& storage_category, 
    const std::string& storage_sub, 
    const std::string& status, 
    const std::string& remarks,
    const std::string& date_received,
    const std::string& date_released,
    const std::string& released_to,
    const std::string& username
) {
    try {
        mysqlx::Table inventory = instance().session.getSchema(SQLConsts::dbName).getTable("inventory");
        mysqlx::Result result = inventory.insert(SQLColumn::EQ_NAME, SQLColumn::EQ_SERIAL_NUM, SQLColumn::EQ_QUANTITY, SQLColumn::EQ_UNIT_ID, SQLColumn::EQ_LOCATION, SQLColumn::EQ_STORAGE, SQLColumn::EQ_STORAGE_SUB, SQLColumn::EQ_STATUS, SQLColumn::EQ_REMARKS, SQLColumn::EQ_D_RECEIVED, SQLColumn::EQ_D_RELEASED, SQLColumn::EQ_RELEASED_P)
            .values(product, serial_num, quantity, unit_id, location, storage_category, storage_sub, status, remarks, date_received, date_released, released_to).execute();

        Logs::logLine(Logs::Type::Operation, Logs::CrudOperation::Create, username, product, result.getAutoIncrementValue());

        return MySQLResult::Success;
    }
    catch (const mysqlx::Error& err) {
        Logs::logLine(Logs::Type::Error, Logs::CrudOperation::Read, "", "", -1, err.what());
        return MySQLResult::InternalServerError;
    }
}


mysqlx::RowResult MySQLManager::viewEquipment(const std::string& username,const std::string& storage) {
    try {
        mysqlx::Schema db = instance().session.getSchema("lantaka_ims");
        mysqlx::Table inventory = db.getTable("inventory");

        Logs::logLine(Logs::Type::Operation, Logs::CrudOperation::Read, username, storage);

        return inventory.select("*").where(SQLColumn::EQ_STORAGE + " = '" + storage + "'").execute();
    }
    catch (const mysqlx::Error& err) {
        Logs::logLine(Logs::Type::Error, Logs::CrudOperation::Read, "", "", -1, err.what());
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
        Logs::logLine(Logs::Type::Error, Logs::CrudOperation::Read, "", "", -1, err.what());
        return std::string();
    }
}


MySQLResult MySQLManager::deleteEquipment(const int& inv_id, const std::string& username) {
    try {
        mysqlx::Schema IMS = instance().session.getSchema(SQLConsts::dbName);
        mysqlx::Table inventory = instance().session.getSchema(SQLConsts::dbName).getTable("inventory");

        std::string equipmentName = getEquipmentUsername(inv_id);
        Logs::logLine(Logs::Type::Operation, Logs::CrudOperation::Delete, username, equipmentName, inv_id); // Has to be logged before the delete query

        inventory.remove()
            .where("I_ID = :inv_id")
            .bind("inv_id", inv_id)
            .execute();

        return MySQLResult::Success;
    }
    catch (const mysqlx::Error& err) {
        Logs::logLine(Logs::Type::Error, Logs::CrudOperation::Read, "", "", -1, err.what());
        return MySQLResult::InternalServerError;
    }
}