#include <cstring>
#include <iostream>

#include <signal.h>

#include "Server.h"
#include "Conn.h"

#include "event2/listener.h"


Server::Server(std::string address) : packets(0), bytes(0), startTime(0), shutdown(false), address(address) {}


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
	    0, -1,
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
    return event_base_dispatch(base) == 0;
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
    if (base != nullptr) {
        event_base_free(base);
    }
    if (signalEvent != nullptr) {
        event_free(signalEvent);
    }
    if (listener != nullptr) {
        evconnlistener_free(listener);
    }
}

void Server::IncrBytes(uint64_t byte) {
    bytes += byte;
}

void Server::IncrPacket() {
    packets += 1;
}