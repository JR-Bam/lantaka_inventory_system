
# Automatic Inventory System

This is an automatic Inventory System developed to keep track of equipment.


## Tech Stack

**Client:** Bootstrap, Fetch API

**Server:** C++ REST API 

### Libraries Used
- [nlohmann/json](https://github.com/nlohmann/json)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- [jwt-cpp](https://github.com/Thalhammer/jwt-cpp)
- [MySQL Connector C++ JDBC API (for XAMPP)](https://dev.mysql.com/doc/connector-cpp/1.1/en/connector-cpp-examples-connecting.html)
- [MySQL Connector C++ XDevAPI (for MySQL X)](https://dev.mysql.com/doc/x-devapi-userguide/en/)


## Run Locally

Clone the project

```bash
  git clone https://github.com/JR-Bam/lantaka_inventory_system
```

Go to the project directory

```bash
  cd my-project
```

Create a db/schema named `"db"` and import the `db.sql` file in phpmyadmin.



Open the project in Visual Studio 2022, then build and run in **Release** mode.

> [!TIP]
> You don't need to do library linking anymore, as it's already been implemented in this project. If you want to use the respective libraries, just include the respective headers. 

- [nlohmann/json](https://github.com/nlohmann/json) - `#include "json.hpp"`
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) - `#include "httplib.h"`
- [JDBC MySQL Driver](https://dev.mysql.com/doc/x-devapi-userguide/en/) - `#include "mysql/jdbc.h" `


> [!NOTE]
> [`server_shutdown.exe`](https://github.com/JR-Bam/lantaka_inventory_system/blob/master/server_shutdown.exe) is the one you need to run to turn off the server. It's a program I made that simply calls an HTTP GET request onto our server to shut it down, the source code is not included in this project so hit me up if you want to see it. So the main flow of running and building it is to do the aforementioned steps and if you want to shut down the server either do it in Visual Studio or run the exe file.

## MySQL JDBC Mini Walkthrough

Taken from the [MySQL Connector/C++](https://dev.mysql.com/doc/connector-cpp/1.1/en/connector-cpp-getting-started-examples.html) page.

#### # Connecting to MySQL
To establish a connection to the MySQL server, retrieve an instance of `sql::Connection` from a `sql::mysql::MySQL_Driver` object. A `sql::mysql::MySQL_Driver` object is returned by `sql::mysql::get_mysql_driver_instance()`. 

```cpp
sql::mysql::MySQL_Driver *driver;
sql::Connection *con;

driver = sql::mysql::get_mysql_driver_instance();
con = driver->connect("tcp://127.0.0.1:3306", "user", "password");

delete con;

```

Make sure that you free `con`, the `sql::Connection` object, as soon as you do not need it any more. But do not explicitly free `driver`, the connector object. Connector/C++ takes care of freeing that.

#### # Running a Simple Query
To run simple queries, you can use the `sql::Statement::execute()`, `sql::Statement::executeQuery()`, and `sql::Statement::executeUpdate()` methods. Use the method `sql::Statement::execute()` if your query does not return a result set or if your query returns more than one result set.
```cpp
sql::mysql::MySQL_Driver *driver;
sql::Connection *con;
sql::Statement *stmt;

driver = sql::mysql::get_mysql_driver_instance();
con = driver->connect("tcp://127.0.0.1:3306", "user", "password");

stmt = con->createStatement();
stmt->execute("USE " EXAMPLE_DB);
stmt->execute("DROP TABLE IF EXISTS test");
stmt->execute("CREATE TABLE test(id INT, label CHAR(1))");
stmt->execute("INSERT INTO test(id, label) VALUES (1, 'a')");

delete stmt;
delete con;
```

> [!Note]
> You must free the `sql::Statement` and `sql::Connection` objects explicitly using delete. 

#### # Fetching Results
The API for fetching result sets is identical for (simple) statements and prepared statements. If your query returns one result set, use `sql::Statement::executeQuery()` or `sql::PreparedStatement::executeQuery()` to run your query. Both methods return `sql::ResultSet` objects. 

```cpp
// ...
sql::Connection *con;
sql::Statement *stmt;
sql::ResultSet  *res;
// ...
stmt = con->createStatement();
// ...

res = stmt->executeQuery("SELECT id, label FROM test ORDER BY id ASC");
while (res->next()) {
  // You can use either numeric offsets...
  cout << "id = " << res->getInt(1); // getInt(1) returns the first column
  // ... or column names for accessing results.
  // The latter is recommended.
  cout << ", label = '" << res->getString("label") << "'" << endl;
}

delete res;
delete stmt;
delete con;
```

> [!Note]
> In the preceding code snippet, column indexing starts from 1. 

> [!Note]
> You must free the `sql::Statement`, `sql::Connection`, and `sql::ResultSet` objects explicitly using `delete`.

#### # Using Prepared Statements
`sql::PreparedStatement` is created by passing an SQL query to `sql::Connection::prepareStatement()`. As `sql::PreparedStatement` is derived from `sql::Statement`, you will feel familiar with the API once you have learned how to use (simple) statements (`sql::Statement`). For example, the syntax for fetching results is identical. 

```cpp
// ...
sql::Connection *con;
sql::PreparedStatement  *prep_stmt
// ...

prep_stmt = con->prepareStatement("INSERT INTO test(id, label) VALUES (?, ?)");

prep_stmt->setInt(1, 1);
prep_stmt->setString(2, "a");
prep_stmt->execute();

prep_stmt->setInt(1, 2);
prep_stmt->setString(2, "b");
prep_stmt->execute();

delete prep_stmt;
delete con;
```

> [!Note]
> You must free the `sql::PreparedStatement` and `sql::Connection` objects explicitly using `delete`.

#### # Error Handling
Error handling is done using exceptions that derive from `sql::SQLException` class that is derived from `std::runtime_error`.

```cpp
catch (sql::SQLException &e)
{
  /*
    The JDBC API throws three different exceptions:

  - sql::MethodNotImplementedException (derived from sql::SQLException)
  - sql::InvalidArgumentException (derived from sql::SQLException)
  - sql::SQLException (derived from std::runtime_error)
  */

  cout << "# ERR: SQLException in " << __FILE__;
  cout << "(" << "EXAMPLE_FUNCTION" << ") on line " << __LINE__ << endl;

  /* Use what() (derived from std::runtime_error) to fetch the error message */

  cout << "# ERR: " << e.what();
  cout << " (MySQL error code: " << e.getErrorCode();
  cout << ", SQLState: " << e.getSQLState() << " )" << endl;

  return EXIT_FAILURE;
}

```

#### # Full Example
```cpp
/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>

/*
  Include the MySQL library
*/
#include "mysql/jdbc.h"

using namespace std;

int main(void)
{
  cout << endl;
  cout << "Let's have MySQL count from 10 to 1..." << endl;

  try {
    sql::Driver *driver;
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;
    sql::PreparedStatement *pstmt;

    /* Create a connection */
    driver = get_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "root", "root");
    /* Connect to the MySQL test database */
    con->setSchema("test");

    stmt = con->createStatement();
    stmt->execute("DROP TABLE IF EXISTS test");
    stmt->execute("CREATE TABLE test(id INT)");
    delete stmt;

    /* '?' is the supported placeholder syntax */
    pstmt = con->prepareStatement("INSERT INTO test(id) VALUES (?)");
    for (int i = 1; i <= 10; i++) {
      pstmt->setInt(1, i);
      pstmt->executeUpdate();
    }
    delete pstmt;

    /* Select in ascending order */
    pstmt = con->prepareStatement("SELECT id FROM test ORDER BY id ASC");
    res = pstmt->executeQuery();

    /* Fetch in reverse = descending order! */
    res->afterLast();
    while (res->previous())
      cout << "\t... MySQL counts: " << res->getInt("id") << endl;
    delete res;

    delete pstmt;
    delete con;

  } catch (sql::SQLException &e) {
    cout << "# ERR: SQLException in " << __FILE__;
    cout << "(" << __FUNCTION__ << ") on line " »
      << __LINE__ << endl;
    cout << "# ERR: " << e.what();
    cout << " (MySQL error code: " << e.getErrorCode();
    cout << ", SQLState: " << e.getSQLState() << »
      " )" << endl;
  }

  cout << endl;

  return EXIT_SUCCESS;
}
```
