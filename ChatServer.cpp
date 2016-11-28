//
// Created by WebSter on 28.11.2016.
//

#include <iostream>
#include "ChatServer.h"

ChatServer::ChatServer(SOCKET s) {
    m_Socket = s;
}

int ChatServer::login(string username) {
    // username.size() is guaranteed to be less than 33
    string msg = username;
    msg.insert(0, 1, (char) CODE_LOGINREQUEST);
    uint16_t len = (uint16_t) htons((u_short) username.size());
    msg.insert(1, (char*) &len, 2);

    int r = send(m_Socket, msg.c_str(), (int) msg.size(), 0);
    if(r == SOCKET_ERROR) {
        printf("Failed to login with error: %s\n", strerror(r));
        return -1;
    }
    if(r != msg.size()) {
        printf("Login: Not all bytes are sent!");
        return -2;
    }
    return 0;
}