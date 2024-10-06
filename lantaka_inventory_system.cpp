// lantaka_inventory_system.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <httplib.h>

using namespace httplib;

void helloworld(const Request& req, Response& res) {
    res.set_content("Hello, World!", "text/plain");
}

int main()
{
    Server svr;

    svr.set_base_dir("./public");

    svr.Get("/home/this", &helloworld);

    std::cout << "Starting server..." << std::endl;
    svr.listen("localhost", 8080);

    return 0;
}

