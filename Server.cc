#include <cstring>
#include <iostream>

#include <signal.h>

#include "Server.h"
#include "Conn.h"

#include "event2/listener.h"


Server::Server(std::string address) : 
        base(nullptr), listener(nullptr), signalEvent(nullptr), timerEvent(nullptr),
        packets(0), bytes(0), startTime(0), shutdown(false), address(address) {}


bool Server::Start() {
    if (shutdown) {
        return false;
    }
    base = event_base_new();
    if (base == nullptr) {
        return false;
    }
    struct sockaddr_in sin;
    int sinSize = sizeof(sin);
    memset(&sin, 0, sinSize);
    if (0 != evutil_parse_sockaddr_port(address.c_str(), (struct sockaddr*)&sin, &sinSize)) {
        return false;
    }
    listener = evconnlistener_new_bind(base, &Server::Accept, (void *)this,
	    LEV_OPT_REUSEABLE, 15,
	    (struct sockaddr*)&sin,
	    sizeof(sin));
    if (listener == nullptr) {
        return false;
    }
    signalEvent = evsignal_new(base, SIGINT, &Server::Signal, (void *)this);
    if (signalEvent == nullptr) {
        return false;
    }
    event_add(signalEvent, nullptr);
    timerEvent = evtimer_new(base, &Server::OnTimer, (void*)this);
    if (timerEvent == nullptr) {
        return false;
    }
    AddTimer();
    return event_base_dispatch(base) == 0;
}

void Server::AddTimer() {
    if (timerEvent != nullptr) {
        struct timeval tv = {.tv_sec = 1, .tv_usec = 0 };
        event_add(timerEvent, &tv);
    }
}

void Server::Accept(evconnlistener *listener, int sock, sockaddr *addr, int len, void *ptr) {
    std::cout << sock << std::endl;
    Server *server = static_cast<Server*>(ptr);
    std::shared_ptr<Conn> conn = std::make_shared<Conn>(server, sock, MAX_BUFFER);
    if (!conn->Init()) {
        // log
        return;
    }
    server->Register(sock, conn);
}

struct event_base *Server::GetBase() {
    return base;
}

void Server::Shutdown() {
    if (shutdown) {
        return;
    }
    shutdown = true;
    std::cout << "Shutdown" << std::endl;
    if (listener != nullptr) {
        evconnlistener_disable(listener);
    }
    if (signalEvent != nullptr) {
        event_del(signalEvent);
    }
    if (timerEvent != nullptr) {
        event_del(timerEvent);
    }
    for (auto &conn : conns) {
        conn.second->OnClose();
    }
    if (base != nullptr) {
        struct timeval delay = { 0, 0 };
	    event_base_loopexit(base, &delay);
    }
}

void Server::Signal(evutil_socket_t sock, short what, void *ptr) {
    Server *server = static_cast<Server*>(ptr);
    server->Shutdown();
}


void Server::Register(int sock, std::shared_ptr<Conn> conn) {
    conns[sock] = conn;
}


void Server::UnRegister(int sock) {
    conns.erase(sock);
}

Server::~Server() {
    Shutdown();

    if (signalEvent != nullptr) {
        event_free(signalEvent);
    }
    if (listener != nullptr) {
        evconnlistener_free(listener);
    }
    if (timerEvent != nullptr) {
        event_free(timerEvent);
    }
    if (base != nullptr) {
        event_base_free(base);
    }
}

void Server::IncrBytes(uint64_t byte) {
    bytes += byte;
}

void Server::IncrPacket() {
    packets += 1;
}

void Server::OnTimer(int, short what, void *ptr) {
    Server *server = static_cast<Server*>(ptr);
    uint64_t packets = server->packets.exchange(0);
    uint64_t bytes = server->bytes.exchange(0);
    std::cout << packets << "/s" << std::endl;
    std::cout << bytes / 1024 << "kb/s" << std::endl;
    server->AddTimer();
}