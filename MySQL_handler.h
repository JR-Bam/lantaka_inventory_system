#pragma once
#include <mysql/jdbc.h>

namespace SQLConsts {
	const std::string username = "root";
	const std::string password = "";
	const std::string dbName = "db";
}

namespace SQLColumn {
	const std::string EQ_ID = "I_ID";
	const std::string EQ_NAME = "I_Product";
	const std::string EQ_SERIAL_NUM = "I_SN";
	const std::string EQ_QUANTITY = "I_Quantity";
	const std::string EQ_UNIT_ID = "UNIT_ID";
	const std::string EQ_LOCATION = "I_Location";
	const std::string EQ_STORAGE = "E_Storage";
}

class MySQL_handler
{
public:
	static sql::Connection* connectDB();
};

