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
    void Register(int sock, std::shared_ptr<ClientWriter> writer);
    void UnRegister(int sock);
    const std::string &ServerAddress();
    struct event_base *GetBase();
    bool Shutdown();
    ~Client();
private:
    struct event_base *base;
    std::string serverAddress;
    int packetSize;
    int concurrent;
    std::unordered_map<int, std::shared_ptr<ClientWriter>> clientWriters;
};

class ClientWriter {
public:
    ClientWriter(Client *client, int packetSize) : client(client), sock(-1), packetSize(packetSize), connected(false) {}
    bool Init();
    ~ClientWriter();
private:
    Client *client;
    int sock;
    int packetSize;
    std::vector<char> buffer;
    int writeIndex;
    struct event *event;
    bool    connected;
public:
    static void OnConnect(int sock, short int what, void *ptr);
    static void OnEvent(int sock, short int what, void *ptr);
};

