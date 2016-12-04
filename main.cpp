#ifdef __cplusplus
extern "C" {
#endif
char* program_name;
#ifdef __cplusplus
}
#endif

#include <iostream>
#include <queue>
#include "etcp.h"
#include "ChatServer.h"
#include "automutex.h"

using namespace std;

ChatServer chatServer;
queue<string> MessagesToSend;
static CAutoMutex autoMutex;
volatile bool bgThreadAlive;

DWORD WINAPI BackgroundThread(LPVOID data) {
    SOCKET s = (SOCKET) data;
    string msg;
    unsigned char code = 255;
    int rcvdb;
    bool ok = true;

    while(ok) {
        if (!MessagesToSend.empty()) {
            SCOPE_LOCK_MUTEX(autoMutex.get());
            if (!MessagesToSend.empty()) {
                msg = MessagesToSend.front();
                int r = chatServer.sendMessage(msg);
                if (r < 0) {
                    cout << "Failed to send message!" << endl;
                } else {
                    MessagesToSend.pop();
                }
            }
        }
        // Receive messages
        rcvdb = readn(s, (char*) &code, 1);
        if (rcvdb == 0) {
            break;
        } else if(rcvdb < 0) {
            cerr << "Error: reading from socket " << s << endl;
            break;
        } else {
            switch(code) {
                case CODE_OUTMSG: // Incoming message from some client

                    break;
                case CODE_SRVERR:
                    ok = false;
                case CODE_SRVMSG:
                    cout << "[ Message from server ]" << endl;
                    cout << chatServer.receiveServerMessage();
                    cout << "[ End of message ]" << endl;
                    break;
                case CODE_FORCEDLOGOUT:
                    cout << "[ You were forced to logout by server ]" << endl;
                    ok = false;
                    break;
                case CODE_LOGINNOTIFY:
                    if(chatServer.receiveLoginNotification()) {
                        cout << "[ " << chatServer.getFullName(
                                chatServer.getCurrentPeer()) << " has come online ]";
                    }
                    break;
                case CODE_LOGOUTNOTIFY:
                    if(chatServer.receiveLogoutNotification()) {
                        cout << "[ " << chatServer.getFullName(
                                chatServer.getCurrentPeer()) << " has gone offline ]";
                    }
                    break;
                case CODE_HEARTBEAT:

                    break;
                default:
                    break;
            }
        }
    }
    //...
    bgThreadAlive = false;
    return 0;
}

int main(int argc, char** argv) {
    //struct sockaddr_in peer;
    SOCKET s;
    string username, peername, name;

    INIT();

    s = tcp_client(argv[1], argv[2]);
    chatServer = ChatServer(s);

    try {
        // Log in
        cout << "What is your name?" << endl;
        while (1) {
            cin >> username;
            if (username == "/quit") throw Exception();
            if (username.length() <= MAX_USERNAME_LENGTH) {
                if (username.find('#') != string::npos) {
                    cout << "Symbol # is prohibited to use in names!" << endl;
                } else break;
            }
            cout << "This name is too long! It should be at most " <<
            MAX_USERNAME_LENGTH << " characters long!";
        }
        chatServer.login(username);
        if (chatServer.receiveUsersList() == 0) {
            cout << "You are successfully logged in!" << endl;
            chatServer.showUsersList();
        }

        HANDLE bgThread = CreateThread(NULL, 0, BackgroundThread, (LPVOID) s, 0, NULL);
        if(bgThread == NULL) {
            cerr << "CANNOT START THREAD" << endl;
            throw Exception();
        }
        bgThreadAlive = true;
        while(bgThreadAlive) {
            /* NO SOCKET OPERATIONS HERE */
            cout << "Type name or id (with # symbol) of user you wish to chat with:" << endl;
            while (bgThreadAlive) {
                cin >> name;
                if (name == "/quit") throw Exception();
                if (name == "/refresh") {
                    chatServer.showUsersList();
                    continue;
                }
                if (name.length() <= MAX_USERNAME_LENGTH) {
                    peername = chatServer.startChat(name);
                    if (!peername.empty()) {
                        break;
                    } else {
                        cout << "Cannot find user \"" << name << "\"" << endl;
                        cout << "Try to type \"/refresh\" to check if this user is still online" << endl;
                    }
                }
                else
                    cout << "This name is too long! It should be at most " <<
                    MAX_USERNAME_LENGTH << " characters long!";
            }
            if(!bgThreadAlive) break;
            system("cls");
            cout << "Chat with " << peername << ":" << endl << endl;
            string str;
            while (bgThreadAlive) {
                cin >> str;
                if (str == "/quit") throw Exception();
                if (str == "/bye") break;
                {
                    SCOPE_LOCK_MUTEX(autoMutex.get());
                    MessagesToSend.push(str);
                }
            }
            // Dialogue leaving process
            // ...
        }
    } catch (Exception& ex) {
        bgThreadAlive = false;
        // Join bgThread?
    }
    cout << "Disconnecting from server..." << endl;
    shutdown(s, SD_BOTH);
    cout << "Closing socket and exiting..." << endl;
    closesocket(s);
    EXIT(0);
}