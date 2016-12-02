//
// Created by WebSter on 28.11.2016.
//

#include <iostream>
#include <list>
#include "ChatServer.h"

ChatServer::ChatServer(SOCKET s) {
    m_Socket = s;
}

int ChatServer::login(string username) {
    // username.size() is guaranteed to be less than 33
    string msg = username;
    uint16_t len = (uint16_t) htons((u_short) username.size());
    msg.insert(0, 1, (char) CODE_LOGINREQUEST);
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

int ChatServer::receiveUsersList() {
    char code;
    char id_buf [10]; //max 10 digits in a 32-bit id
    char name_buf [MAX_USERNAME_LENGTH];
    int id;
    int bnum;

    readn(m_Socket, &code, 1);
    if(code == CODE_LOGINANSWER) {
        while(1) {
            bnum = readline(m_Socket, id_buf, sizeof(id_buf));
            if (bnum < 0) {
                cout << "UsersList packet corrupted: no id" << endl;
                return -1;
            } else if (strcmp(id_buf,"\4\n") == 0) {
                break;
            }
            id = atoi(id_buf);
            bnum = readline(m_Socket, name_buf, sizeof(name_buf));
            if (bnum <= 0) {
                cout << "UsersList packet corrupted: no name for id" << endl;
                return -1;
            }
            name_buf[bnum-1] = '\0';
            m_Users[id] = string(name_buf);
        }
    } else {
        cout << "Inappropriate receiveUsersList() call." << endl;
        return -1;
    }
    return 0;
}

void ChatServer::showUsersList() {
    cout << m_Users.size() << " user(s) online:" << endl;
    list<string> vs;
    unordered_map<int, string>::const_iterator it;
    for (it = m_Users.cbegin(); it != m_Users.cend(); it++) {

        vs.push_back(it->second + "#" + to_string(it->first));
    }
    vs.sort();
    for(string str : vs) {
        cout << str << endl;
    }
}

string ChatServer::startChat(string peer) {
    unordered_map<int, string>::iterator it;
    if(peer.find('#') != string::npos) {
        //Search for the id
        string str_id = peer.substr(1);
        int id = atoi(str_id.c_str());
        it = m_Users.find(id);
    } else { //Search for the name
        for (it = m_Users.begin(); it != m_Users.end(); it++) {
            if (it->second == peer) break;
        }
    }
    return (it == m_Users.end() ? "" : (it->second + "#" + to_string(it->first)));
}

int ChatServer::sendMessage(string msg) {

    return 0;
}

// Parsing for old format of UsersList packet
//    char buf [MAX_USERSONLINE*36];

//        int rdbytes = readvrec(m_Socket, buf, MAX_USERSONLINE*36);
//        if(rdbytes == -1) {
//            printf("Read error\n");
//        } else if(rdbytes == 0) {
//            printf("Not all read\n");
//        } else if(rdbytes % 36 != 0) {
//            printf("Packet corrupted\n");
//        }
//        int N = rdbytes / 36;
//        // Now parse the string
//        for(int j = 0; j < 36*N; j += 36) {
//            char cID[4];
//            int i = j;
//            for (; i < j + 4; ++i) {
//                cID[i] = buf[i];
//            }
//            unsigned int ID = *((unsigned int *) cID);
//            char name[32];
//            for (; i < j + 36; ++i) {
//                name[i - 4] = buf[i];
//            }
//            m_Users[ID] = string(name);
//        }