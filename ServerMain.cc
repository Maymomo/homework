#include "Server.h"
#include <iostream>

int main(void) {
    Server server("0.0.0.0:8080");
    server.Start();
    std::cout << "xxx" << std::endl;
    return 0;
}