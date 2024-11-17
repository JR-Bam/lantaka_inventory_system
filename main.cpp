// lantaka_inventory_system.cpp : This file contains the 'main' function. Program execution begins and ends there.
// !!!!!! Run in Release MODE !!!!!!

#include "RouteHandlers.h"

#include <thread>
#include <chrono>
#include <iostream>

static void shutdown_server(Server& svr) {
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
    svr.Get("/api/equipment/view", &RouteHandlers::viewEquipment);
    svr.Post("/api/equipment/add", &RouteHandlers::addEquipment);
    svr.Post("/api/equipment/edit", &RouteHandlers::editEquipment);
    svr.Delete(R"(/api/equipment/delete/(\d+))", &RouteHandlers::removeEquipment);

    svr.Post("/api/signup", &RouteHandlers::signUp);
    svr.Post("/api/login", &RouteHandlers::verifyAccount);
    svr.Get("/api/validate-token", &RouteHandlers::authSession);
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

