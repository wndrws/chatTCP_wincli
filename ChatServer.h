//
// Created by WebSter on 28.11.2016.
//

#pragma once

#include "etcp.h"
#include <string>

#define CODE_SRVMSG 0
#define CODE_LOGINREQUEST 1
#define CODE_LOGOUTREQUEST 2

using namespace std;

class ChatServer {
private:
    SOCKET m_Socket;
public:
    ChatServer(SOCKET);
    int login(string username);
};
