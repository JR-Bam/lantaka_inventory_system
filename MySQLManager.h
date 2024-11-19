#pragma once
#include <mysqlx/xdevapi.h>
#include <utility>

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

/**
 * @enum MySQLResult
 * @brief Represents the result of a MySQL operation.
 *
 * This enumeration defines possible outcomes of a MySQL operation, such as
 * authentication or querying the database.
 */
enum class MySQLResult {
	/// The operation was successful (e.g., valid credentials).
	Success,
	/// Invalid username or password.
	BadCredentials,
	/// The requested resource or data was not found.
	NotFound,
	/// Internal server error, an exception was caught.
	InternalServerError
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

	static std::string			getEquipmentUsername	(int id);
public:

	/**
	 * Checks if the provided username and password combination is valid by querying the users table 
	 * in the MySQL database.
	 *
	 * @param username the username provided by the client
	 * @param password the password provided by the client
	 * 
	 * @return MySQLResult can have the following possible values:
	 * - MySQLResult::Success: Returned if the credentials are valid.
	 * - MySQLResult::BadCredentials: Returned if the username does not exist in the database or if the password is incorrect.
	 * - MySQLResult::InternalServerError: Returned if a mysqlx::Error is caught during execution.
	 */
	static MySQLResult			validateCredentials		(const std::string& username, 
														 const std::string& password);

	/**
	 * Updates specified fields of an equipment entry in the inventory table of the MySQL database 
	 * based on the provided equipment ID. If the query is successful, this action is logged.
	 *
	 * @param id the id of the equipment
	 * @param params vector of pair of strings that represent a list of keys (column name) and values (updated value)
	 * @param username the username of the user responsible for accessing the endpoint that calls this function
	 *
	 * @return MySQLResult can have the following possible values:
	 * - MySQLResult::Success: Returned if the update was successful, with at least one row affected.
	 * - MySQLResult::NotFound: Returned if no rows were affected, indicating the specified ID does not exist.
	 * - MySQLResult::InternalServerError: Returned if a mysqlx::Error is caught during execution.
	 */
	static MySQLResult			updateEquipment			(const int id, 
														 const std::vector<std::pair<std::string, std::string>>& params, 
														 const std::string& username);

	static MySQLResult			signUp					(const std::string& username, 
														 const std::string& password);

	static MySQLResult			addEquipment			(const std::string& product, 
														 const std::string& serial_num, 
														 int quantity, 
														 int unit_id, 
														 const std::string& location, 
														 const std::string& storage, 
														 const std::string& username);

	static mysqlx::RowResult	viewEquipment			(const std::string& username);
	static std::string			queryEquipment			(const int& unit_id);
	static MySQLResult			deleteEquipment			(const int& inv_id, 
														 const std::string& username);
};

