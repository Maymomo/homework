#include <string>

#include "event.h"

#include "Parser.h"

class Client {
public:
    Client(std::string serverAddress, int size, int concurrent) {}
    ~Client() {}
private:
    struct event_base *base;
};

int main(int argc, char **argv) {
    return 0;
}