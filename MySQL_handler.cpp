#include "MySQL_handler.h"

sql::Connection* MySQL_handler::connectDB()
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
