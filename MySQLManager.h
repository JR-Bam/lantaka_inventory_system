#pragma once
#include <mysql/jdbc.h>
#include <mysqlx/xdevapi.h>

// Change these accordingly
namespace SQLConsts {
	const std::string username = "user";
	const std::string password = "";
	const std::string dbName = "lantaka_ims";
}

namespace SQLColumn {
	const std::string EQ_ID = "I_ID";
	const std::string EQ_NAME = "I_Product";
	const std::string EQ_SERIAL_NUM = "I_SN";
	const std::string EQ_QUANTITY = "I_Quantity";
	const std::string EQ_UNIT_ID = "UNIT_ID";
	const std::string EQ_LOCATION = "I_Location";
	const std::string EQ_STORAGE = "E_Storage";

	const std::string USER_UNAME = "U_Username";
	const std::string USER_PWORD = "U_Password";
}

enum class LoginResult {
	Success,            // Valid credentials
	BadCredentials,     // Invalid username or password
	InternalServerError // Server error, exception caught
};

class MySQLManager
{
private:
	mysqlx::Session session;

	// Disable copy and assignment
	MySQLManager(const MySQLManager&) = delete;
	MySQLManager& operator=(const MySQLManager&) = delete;

	// Singleton instance retrieval
	static MySQLManager& instance() {
		static MySQLManager manager;
		return manager;
	}

	// Constructor
	MySQLManager()
		: session(
			mysqlx::SessionOption::HOST, "localhost",
			mysqlx::SessionOption::PORT, 33060,
			mysqlx::SessionOption::USER, SQLConsts::username,
			mysqlx::SessionOption::PWD, SQLConsts::password
		) {}

	// Destructor 
	~MySQLManager() {
		session.close();
	}
public:
	// Legacy function, outdated.
	static sql::Connection* connectDB();


	// Not recommended for RouteHandlers to work with sql directly, I recommend making 
	// functions built for a specific task that returns a mysqlx::RowResult and RouteHandlers
	// will only need to parse the RowResult.
	// FYI: You don't need to free this
	static mysqlx::Schema* getDatabase(const std::string& name);

	static LoginResult isValidCredentials(const std::string& username, const std::string& password);
};

