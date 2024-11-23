// lantaka_inventory_system.cpp : This file contains the 'main' function. Program execution begins and ends there.
// !!!!!! Run in Release MODE !!!!!!


#include "Logs.h"
#include "RouteHandlers.h"

#include <thread>
#include <chrono>
#include <iostream>
#include <windows.h>

static const std::string HOST = "localhost";
static const int PORT = 8080;


static void shutdown_server(Server& svr) {
    // Wait for 3 seconds before stopping
    std::this_thread::sleep_for(std::chrono::seconds(3));
    svr.stop();
    
    Logs::logLine(Logs::Type::Stop);
}

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

    svr.Post("/api/signup", &RouteHandlers::signUp);
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

