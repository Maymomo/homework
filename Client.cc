#include "Client.h"


bool Client::Start() {
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
    if (0 != connect(sock, &sin, sinSize)) {
        if (errno != EINPROGRESS) {
            return false;
        }
        event = event_new(client->GetBase(), sock, EV_WRITE | EV_PERSIST, &ClientWriter::OnConnect， (void*)(this));
        if (event == nullptr) {
            return false;
        }
        event_add(event, NULL);
    }
    return truek;
}