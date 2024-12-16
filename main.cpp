/*****************************************************************//**
 * \file   main.cpp
 * \brief  This file contains the 'main' function. Program execution begins and ends there.
 * 
 * \date   November 2024
 *********************************************************************/

/** \mainpage Lantaka Inventory Management System
 * This is the documentaion for the Lantaka Inventory Management System.
 * Information about classes and functions can be found here.
 * 
 * \section compiling Compiling the program.
 * 
 * Currently the project supports two configurations and platforms.
 * 1. x86 Release
 * 2. x64 Release
 * 
 * Build the project accordingly. Then, run the curresponding batch scripts to package the build. 
 * 
 * \section dep Dependencies.
 * 
 * The program depends on a mysql server set up with the X protocol. This program will utilize
 * a MySQL client with the following credentials:
 * - Username: user
 * - Password (empty): 
 * - Schema Name: lantaka_ims
 * 
 * \section lib Libraries Used.
 * This program utilizes the following external libraries:
 * - [nlohmann/json](https://github.com/nlohmann/json)
 * - [cpp-httplib](https://github.com/yhirose/cpp-httplib)
 * - [jwt-cpp](https://github.com/Thalhammer/jwt-cpp)
 * - [MySQL Connector C++ XDevAPI](https://dev.mysql.com/doc/x-devapi-userguide/en/)
 * - [libbcrypt](https://github.com/trusch/libbcrypt)
 */

// !!!!!! Run in Release MODE !!!!!!


#include "Logs.h"
#include "RouteHandlers.h"

#include <thread>
#include <chrono>
#include <iostream>
#include <windows.h>

/// The host
static const std::string HOST = "localhost"; 
/// The port
static const int PORT = 8080; 

/**
 * @brief Separate thread function to shut down the server 3 seconds after being called to.
 * 
 * \param svr reference to the Server object.
 */
static void shutdown_server(Server& svr) {
    // Wait for 3 seconds before stopping
    std::this_thread::sleep_for(std::chrono::seconds(3));
    svr.stop();
    
    Logs::logLine(Logs::Type::Stop);
}

/**
 * @brief Separate thread function to open the browser and to the site this server is hosting on.
 * 
 * \param svr reference to the Server object.
 * \param url the url the program redirects the user to
 */
static void open_browser(Server& svr, std::string url) {
    while (!svr.is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::wstring w_url = std::wstring(url.begin(), url.end());
    ShellExecute(NULL, L"open", w_url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) // Don't Ask, this is the main function pramis
{
    Server svr;

    // For HTML Files
    svr.set_mount_point("/", "./public/login");
    svr.set_mount_point("/home", "./public/home");
    svr.set_mount_point("/signup", "./public/signup");
    // For the images
    svr.set_mount_point("/images", "./assets");


    // Endpoints
    svr.Get("/api/equipment/view", &RouteHandlers::viewEquipment);
    svr.Post("/api/equipment/add", &RouteHandlers::addEquipment);
    svr.Put("/api/equipment/edit", &RouteHandlers::editEquipment);
    svr.Delete(R"(/api/equipment/delete/(\d+))", &RouteHandlers::removeEquipment);

    svr.Get("/logs.txt", &RouteHandlers::logs);
    svr.Post("/api/new-pass", &RouteHandlers::newPassword);
    svr.Post("/api/login", &RouteHandlers::verifyAccount);
    svr.Get("/api/validate-token", &RouteHandlers::authSession);
    svr.Get("/shutdown", [&svr](const Request& req, Response& res) {
        res.set_content("Server is shutting down in 3 seconds...", "text/plain");
        // Start the shutdown in a detached thread to avoid blocking the response
        std::thread shutdown_thread(shutdown_server, std::ref(svr));
        shutdown_thread.detach();
    });

    Logs::logLine(Logs::Type::Start);

    // Separate thread to open the browser
    std::thread browser_thread(open_browser, std::ref(svr), std::string("http://" + HOST + ":" + std::to_string(PORT)));

    svr.listen(HOST, PORT);

    browser_thread.join();

    return 0;
}

