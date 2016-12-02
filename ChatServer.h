//
// Created by WebSter on 28.11.2016.
//

#pragma once

#include "etcp.h"
#include <string>
#include <unordered_map>

#define CODE_SRVMSG 0
#define CODE_LOGINREQUEST 1
#define CODE_LOGOUTREQUEST 2
#define CODE_LOGINANSWER 3

#define MAX_USERNAME_LENGTH 32

using namespace std;

class Exception {};

class ChatServer {
private:
    SOCKET m_Socket;
    unordered_map<int, string> m_Users;
    int m_ThisUserID;
public:
    ChatServer(SOCKET);
    int login(string username);
    int receiveUsersList();
    void showUsersList();
    string startChat(string peername);
    int sendMessage(string msg);
};
