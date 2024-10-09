// lantaka_inventory_system.cpp : This file contains the 'main' function. Program execution begins and ends there.
// !!!!!! Run in Release MODE !!!!!!

// P.S. Ako nalang in charge sa jwt_handler shizz kay masakit sa ulo e explain

#include "RouteHandlers.h"

#include <iostream>

void hello(const Request& req, Response& res) {
    res.set_content("Hello, World!", "text/plain");
}

int main()
{
    Server svr;

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

