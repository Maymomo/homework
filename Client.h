#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "event.h"

#include "Parser.h"


class ClientWriter;
class Client {
public:
    Client(std::string serverAddress, int packetSize, int concurrent) : serverAddress(serverAddress), packetSize(packetSize), concurrent(concurrent) {
    }
    bool Start();
    bool Shutdown();
    ~Client();
private:
    struct event_base *base;
    std::string serverAddress;
    int packetSize;
    int concurrent;
    std::unordered_map<int, std::shared_ptr<ClientWriter>> clientWriters;
public:
    static void OnConnect();
};

class ClientWriter {
public:
    ClientWriter(Client *client, int sock, int packetSize) : client(client), sock(sock), packetSize(packetSize) {}
    ~ClientWriter();
private:
    Client *client;
    struct event *event;
    int sock;
    int packetSize;
    std::vector<char> buffer;
    int writeIndex;
};

