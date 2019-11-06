#include <cstring>
#include <unistd.h>
#include "Client.h"


bool Client::Start() {
    base = event_base_new();
    if (base == nullptr) {
        return false;
    }
    for (int i = 0; i < concurrent; i++) {
        std::shared_ptr<ClientWriter> clientWriter = std::make_shared<ClientWriter>(this, packetSize, serverAddress);
        if (!clientWriter->Init()) {
            return false;
        }
        Register(clientWriter->Sock(), clientWriter);
    }
    return 0 == event_base_dispatch(base);
}


struct event_base *Client::GetBase() {
    return base;
}

void Client::Shutdown() {
    for (auto &writer : clientWriters) {
        writer.second->OnClose();
    }
    if (base != nullptr) {
        event_base_loopexit(base, nullptr);
    }
}

void Client::Register(int sock, std::shared_ptr<ClientWriter> writer) {
    clientWriters[sock] = writer;
}

void Client::UnRegister(int sock) {
    clientWriters.erase(sock);
}

void ClientWriter::OnClose() {
    if (event != nullptr) {
        event_del(event);
        event_free(event);
        event = nullptr;
    }
    int sockTmp = sock;
    if (sock >= 0) {
        close(sock);
        sock = -1;
    }
}

void ClientWriter::InitBuffer() {
    buffer.resize(packetSize+4);
    int *headLen = (int*)(buffer.data());
    *headLen = packetSize;
    writeIndex = 0;
}

bool ClientWriter::OnWrite() {
    if (buffer.size() == 0) {
        InitBuffer();
    }
    while (true) {
        int nwrite = write(sock, buffer.data()+writeIndex, buffer.size() - writeIndex);
        if (nwrite == -1) {
            if (errno == EWOULDBLOCK) {
                return true;
            }
            return false;
        }
        writeIndex += nwrite;
        if (writeIndex == buffer.size()) {
            writeIndex = 0;
        }
    }
}


Client::~Client() {
    Shutdown();
    if (base != nullptr) {
        event_base_free(base);
        base = nullptr;
    }
}

ClientWriter::~ClientWriter() {
    OnClose();
}

int ClientWriter::Sock() {
    return sock;
}

bool ClientWriter::Init() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    evutil_make_socket_nonblocking(sock);
    if (sock == -1) {
        return false;
    }
    struct sockaddr_in sin;
    int sinSize = sizeof(sin);
    memset(&sin, 0, sinSize);
    if (0 != evutil_parse_sockaddr_port(address.c_str(), (struct sockaddr*)&sin, &sinSize)) {
        return false;
    }
    if (0 != connect(sock, (struct sockaddr*)&sin, sinSize)) {
        if (errno != EINPROGRESS) {
            return false;
        }
    }
    event = event_new(client->GetBase(), sock, EV_WRITE | EV_PERSIST, &ClientWriter::OnConnect, (void*)(this));
    if (event == nullptr) {
        return false;
    }
    return 0 == event_add(event, NULL);
}

void ClientWriter::OnConnect(int sock, short int what, void *ptr) {
    ClientWriter *clientWriter = static_cast<ClientWriter*>(ptr);
    int optionValue = 0;
    socklen_t optionLen = sizeof(optionValue);
    if (0 != getsockopt(sock, SOL_SOCKET, SO_ERROR, &optionValue, &optionLen)) {
        clientWriter->OnClose();
        return;
    }
    if (optionValue != 0) {
        clientWriter->OnClose();
        return;
    }
    event_del(clientWriter->event);
    event_assign(clientWriter->event, clientWriter->client->GetBase(), sock, EV_WRITE | EV_PERSIST,&ClientWriter::OnEvent, (void*)clientWriter);
    event_add(clientWriter->event, nullptr);
}

void ClientWriter::OnEvent(int sock, short int what, void *ptr) {
    ClientWriter *clientWriter = static_cast<ClientWriter*>(ptr);
    if (what & EV_WRITE) {
        if (!clientWriter->OnWrite()) {
            clientWriter->OnClose();
            clientWriter->client->UnRegister(clientWriter->sock);
        }
    }
}