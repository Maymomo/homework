#include <iostream>

#include <unistd.h>

#include "Conn.h"


Conn::Conn(Server *server, int sock, uint64_t bufferSize) : server(server), sock(sock), bufferSize(bufferSize) {}



void Conn::OnClose() {
    if (event != nullptr) {
        event_del(event);
        event_free(event);
        event = nullptr;
    }
    if (sock >= 0) {
        ::close(sock);
        sock = -1;
    }
    if (server != nullptr) {
        server->UnRegister(sock);
    }
}

Conn::~Conn() {
    OnClose();
}

bool Conn::Init() {
    evutil_make_socket_nonblocking(sock);
    event = event_new(server->GetBase(), sock, EV_READ|EV_PERSIST, Conn::OnEvent, (void*)(this));
    if (event == nullptr) {
        return false;
    }
    return event_add(event, nullptr) == 0;
}


void Conn::OnEvent(int sock, short int what, void *ptr) {
    Conn *conn = static_cast<Conn*>(ptr);
    if (what & EV_READ) {
        if (!conn->OnRead()) {
            conn->OnClose();
            return;
        }
    }
    if (what & EV_WRITE) {
        if (!conn->OnWrite()) {
            conn->OnClose();
            return;
        }
    }
}



bool Conn::OnRead() {
    if (buffer.size() < bufferSize) {
        buffer.resize(bufferSize);
    }
    int readed = 0;
    while (true) {
        if (writeIndex >= buffer.size()) {
            // log
            return false;
        }
        readed = ::read(sock, buffer.data()+writeIndex, buffer.size()-writeIndex);
        if (readed == -1) {
            if (errno == EWOULDBLOCK) {
                return true;
            }
            return false;
        }
        server->IncrBytes(readed);
        writeIndex += readed;
        if (parser.Parse(buffer, writeIndex)) {
            server->IncrPacket();
            writeIndex = 0;
            parser.Reset();
        } else {
            return false;
        }
        if (parser.BodyLen() + parser.HeaderLen() > buffer.size()) {
            return false;
        }
    }
    return true;
}