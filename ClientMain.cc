#include "Client.h"

int main(int argc, char **argv) {
    Client client("127.0.0.1:8080", 1024, 8);
    client.Start();
    return 0;
}