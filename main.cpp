// lantaka_inventory_system.cpp : This file contains the 'main' function. Program execution begins and ends there.
// !!!!!! Run in Release MODE !!!!!!

// P.S. Ako nalang in charge sa jwt_handler shizz kay masakit sa ulo e explain

#include "RouteHandlers.h"

#include <thread>
#include <chrono>
#include <iostream>
#include "mysqlx/xdevapi.h"     // Library for MYSQLX (Because I don's use XAMPP)
#include "mysql/jdbc.h"         // Use this library to connect to XAMPP MySQL

void hello(const Request& req, Response& res) {
    res.set_content("Hello, World!", "text/plain");
}

void shutdown_server(Server& svr) {
    // Wait for 3 seconds before stopping
    std::this_thread::sleep_for(std::chrono::seconds(3));
    svr.stop();
    // TODO: Log into file that server has stopped with timestamp
    std::cout << "Server offline...\n";
}

int main()
{
    Server svr;

    // For HTML Files
    svr.set_mount_point("/", "./public/login");
    svr.set_mount_point("/home", "./public/home");
    // For the images
    svr.set_mount_point("/images", "./assets");

    // Endpoints
    svr.Post("/api/equipment/add", &RouteHandlers::addEquipment);
    svr.Post("/api/login", &RouteHandlers::verifyAccount);
    svr.Get("/api/validate-token", &RouteHandlers::authSession);
    svr.Get("/hello-world", &hello);
    svr.Get("/shutdown", [&svr](const Request& req, Response& res) {
        res.set_content("Server is shutting down in 3 seconds...", "text/plain");
        // Start the shutdown in a detached thread to avoid blocking the response
        std::thread shutdown_thread(shutdown_server, std::ref(svr));
        shutdown_thread.detach();
    });

    std::cout << "Starting server..." << std::endl;
    // TODO: Log into file the time in which server as started
    svr.listen("localhost", 8080);

    return 0;
}

