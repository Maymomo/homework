#pragma once

#include <vector>

#include <event.h>

#include "Server.h"
#include "Parser.h"


class Conn {
public:
    Conn(Server *server, int sock, uint64_t bufferSize);
    bool Init();
    void OnClose();
    ~Conn();
private:
    bool OnRead();
    bool OnWrite() { return false;}
private:
    Server *server;
    struct event *event;
    int sock;
    std::vector<char> buffer;
    int writeIndex;
    uint64_t bufferSize;
    Parser parser;
public:
    static void OnEvent(int sock, short int what, void *ptr);
};