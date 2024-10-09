// lantaka_inventory_system.cpp : This file contains the 'main' function. Program execution begins and ends there.
// !!!!!! Run in Release MODE !!!!!!

// P.S. Ako nalang in charge sa jwt_handler shizz kay masakit sa ulo e explain

#include "RouteHandlers.h"

#include <iostream>
#include "mysqlx/xdevapi.h"

void hello(const Request& req, Response& res) {
    res.set_content("Hello, World!", "text/plain");
}


// Testing out the mysql library. This is just to print all the databases within my mysql server
void testMySQL() {
    using namespace mysqlx;
    try
    {
        Session sesh(
            SessionOption::HOST, "127.0.0.1",
            SessionOption::PORT, 33060,
            SessionOption::USER, "app",
            SessionOption::PWD, ""
        ); // Change these to fit your own mysql server

        for (auto schema : sesh.getSchemas()) {
            std::cout << "Database name: " << schema.getName() << std::endl;
        }

        sesh.close();
    }
    catch (const mysqlx::Error& err) {
        std::cerr << "Error: " << err.what() << '\n';
        exit(1);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        exit(1);
    }

}

int main()
{
    Server svr;

    testMySQL();

    // For HTML Files
    svr.set_mount_point("/", "./public");
    svr.set_mount_point("/home", "./public/home");
    // For the images
    svr.set_mount_point("/images", "./assets");

    // Routes
    svr.Post("/api/login", &RouteHandlers::verifyAccount);
    svr.Get("/api/validate-token", &RouteHandlers::authSession);
    svr.Get("/hello-world", &hello);

    std::cout << "Starting server..." << std::endl;
    svr.listen("localhost", 8080);

    return 0;
}

