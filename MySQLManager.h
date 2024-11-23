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
	const std::string EQ_STORAGE = "Storage_Category";
	const std::string EQ_STORAGE_SUB = "Storage_Subcategory";
	const std::string EQ_STATUS = "Status";

	const std::string USER_UNAME = "U_Username";
	const std::string USER_PWORD = "U_Password";
	const std::string USER_ID = "U_ID";

	const std::string LOGS_TYPE = "M_Type";
	const std::string LOGS_TSTAMP = "M_TimeStamp";
	const std::string LOGS_U_ID = "U_ID";
	const std::string LOGS_E_ID = "E_ID";
	const std::string LOGS_P_TIME = "Parsed_Time";
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

	/**
	 * This function adds a new equipment record to the inventory table in the MySQL database.
	 * It connects to the database, inserts the equipment details into the table,
	 * and logs the operation.
	 *
	 * Preconditions: product, serial_num, quantity, unit_id, location, and storage
	 * must all have valid values. username should be a valid user for logging purposes.
	 *  @return MySQLResult::Success if the equipment was added successfully.
	 * @return MySQLResult::InternalServerError if there was an error during the operation.
	 */
	static MySQLResult			addEquipment			(const std::string& product, 
														 const std::string& serial_num, 
														 int quantity, 
														 int unit_id, 
														 const std::string& location, 
														 const std::string& storage_category, 
														 const std::string& storage_sub, 
														 const std::string& status, 
														 const std::string& username);

	/**
	*This function connects to the mysql database and takes the table inventory from the schema lantaka_ims
	*and returns all inventory that has E_Storage of storage
	* 
	*Preconditions: username should be valid and storage has value
	* 
	*@param username is used for logging the action
	*@param storage to determine which equipments/items to show
	* 
	*@return all inventory items/equipments in specific storage
	*/
	static mysqlx::RowResult	viewEquipment			(const std::string& username,
														 const std::string& storage);
	
	/**
	*This function connects to the mysql database and takes the table unit from the schema lantaka_ims
	*and returns the name of the unit associated with the unit_id in the param
	* 
	*Precondition: unit_id corresponds to a equipment and has value
	* 
	*@param unit_id is the primary key or id of the unit associated with the equipment in this instance
	* 
	*@return the name of the unit
	*/
	static std::string			queryEquipment			(const int& unit_id);
	/**
	*This function connects to the mysql database and takes the table inventory from schema lantaka_ims
	*gets the equipment name by running inv_id in param through getEquipmentUsername() function then deletes it
	* 
	*Precondition: inv_id has value and username is valid
	* 
	*@param inv_id is the id of the equipment to delete
	*@param username is for logging purposes
	* 
	*@return either a success or error
	*/
	static MySQLResult			deleteEquipment			(const int& inv_id, 
														 const std::string& username);

	static MySQLResult			logOp					(std::string& type,
														 std::string& user,
														 int equipmentID,
														 int64_t timestamp,
														 std::string& parsedTime);
};

