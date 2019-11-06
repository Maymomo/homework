#pragma once

#include <atomic>
#include <vector>
#include <memory>
#include <ctime>
#include <string>
#include <unordered_map>

#include <stdint.h>

#include "event.h"

#define MAX_BUFFER  4096

class Conn;

class Server {
public:
    Server(std::string address);
    bool Start();
    void Shutdown();
    struct event_base *GetBase();
    void IncrBytes(uint64_t byte);
    void IncrPacket();
    void Register(int sock, std::shared_ptr<Conn> conn);
    void UnRegister(int sock);
    ~Server();
private:
    struct event_base *base;
    struct evconnlistener *listener;
    struct event        *signalEvent;
    std::atomic<uint64_t>  packets;
    std::atomic<uint64_t>  bytes;
    std::atomic<time_t>  startTime;
    std::atomic<bool>    shutdown;
    std::unordered_map<int, std::shared_ptr<Conn>> conns;
    std::string          address;
public:
    static void Accept(struct evconnlistener *listener,
    evutil_socket_t sock, struct sockaddr *addr, int len, void *ptr);
    static void Signal(evutil_socket_t, short, void *);
};