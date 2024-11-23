
# Automatic Inventory System

This is an automatic Inventory System developed to keep track of equipment.


## Tech Stack

**Client:** Bootstrap, Fetch API

**Server:** C++ REST API 

### Libraries Used
- [nlohmann/json](https://github.com/nlohmann/json)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- [jwt-cpp](https://github.com/Thalhammer/jwt-cpp)
- [MySQL Connector C++ XDevAPI (for MySQL X)](https://dev.mysql.com/doc/x-devapi-userguide/en/)
- [libbcrypt](https://github.com/trusch/libbcrypt)


## Run Locally

Clone the project

```bash
  git clone https://github.com/JR-Bam/lantaka_inventory_system
```

Go to the project directory

```bash
  cd my-project
```

Create a db/schema named `"lantaka_ims"` and import the `db.sql` file in phpmyadmin.

Open the project in Visual Studio 2022, then build and run in **Release** mode.
