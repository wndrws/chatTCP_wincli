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
volatile bool checkOnline = true;
volatile bool checkPending = false;
void showHelp();

HANDLE bgThread;
DWORD WINAPI BackgroundThread(LPVOID data) {
    SOCKET s = (SOCKET) data;
    string msg;
    unsigned char code = 255;
    int rcvdb;
    bool ok = true;
    u_long mode = 0;
    int hbcnt = 0;
    int timeQuantum = 200; // sleep time between readings from non-blocking socket (ms)
    int timeHb = 10000; // time to send the first heartbeat (ms)

    while(ok && bgThreadAlive) {
        if (!MessagesToSend.empty()) {
            SCOPE_LOCK_MUTEX(autoMutex.get());
            if (!MessagesToSend.empty()) {
                msg = MessagesToSend.front();
                int r = chatServer.sendMessage(msg);
                if (r < 0) {
                    cout << "Retrying..." << endl;
                } else {
                    MessagesToSend.pop();
                }
            }
        }
        // Check incoming messages
        if(checkPending && chatServer.getCurrentPeer() == 0
                && !chatServer.getPendingList().empty()) {
            cout << "[ " << "You have incoming message(s) from "
                 << chatServer.getPendingList() << " ]" << endl;
            checkPending = false;
        }
        // Receive packets
        if(mode == 0) {
            mode = 1;
            ioctlsocket(s, FIONBIO, &mode); // Make socket non-blocking
        }
        rcvdb = readn(s, (char*) &code, 1);
        if (rcvdb == 0) {
            cout << "[ Server closed the connection ]" << endl;
            break;
        } else if(rcvdb < 0) {
            int error = WSAGetLastError();
            if(error == WSAEWOULDBLOCK || error == WSAEINTR) {
                // It's ok, continue doing job after some time
                Sleep((DWORD) timeQuantum);
                if(++hbcnt == timeHb/timeQuantum) chatServer.sendHeartbeat();
                else if(hbcnt == 2*timeHb/timeQuantum) chatServer.sendHeartbeat();
                else if(hbcnt == 3*timeHb/timeQuantum) chatServer.sendHeartbeat();
                else if(hbcnt == 4*timeHb/timeQuantum) {
                    cerr << "Error: server is not responding." << endl;
                    break;
                }
            } else {
                cerr << "Error: reading from socket " << s << endl;
                break;
            }
        } else {
            mode = 0;
            ioctlsocket(s, FIONBIO, &mode); // Make socket blocking again
            switch(code) {
                case CODE_OUTMSG: // Incoming message from some client
                    checkPending = chatServer.receiveMessage();
                    chatServer.showMessage(chatServer.getCurrentPeer());
                    break;
                case CODE_SRVERR:
                    ok = false;
                    // no break here
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
                        cout << "[ Your peer has come online ]" << endl;
                    } else if(checkOnline) {
                        if(chatServer.getCurrentPeer() == 0)
                        cout << "[ Some users have come online - "
                             << "type /refresh to see the whole list ]" << endl;
                        checkOnline = false;
                    }
                    break;
                case CODE_LOGOUTNOTIFY:
                    if(chatServer.receiveLogoutNotification()) {
                        cout << "[ Your peer has gone offline ]" << endl;
                    } else if(checkOnline) {
                        if(chatServer.getCurrentPeer() == 0)
                        cout << "[ Some users have gone offline - "
                             << "type /refresh to see the whole list ]" << endl;
                        checkOnline = false;
                    }
                    break;
                case CODE_HEARTBEAT:
                    hbcnt = 0;
                    break;
                default:
                    cerr << "Warning: Packet with unknown code is received!" << endl;
                    break;
            }
        }
    }
    cout << "Press ENTER to exit..." << endl;
    bgThreadAlive = false;
    return 0;
}

int main(int argc, char** argv) {
    //struct sockaddr_in peer;
    SOCKET s;
    string username, peername, name;

    if(argc < 3) {
        cout << "Please provide IP address and Port of the server." << endl;
        return 1;
    }

    INIT();

    s = tcp_client(argv[1], argv[2]);
    chatServer = ChatServer(s);

    try {
        // Log in
        cout << "Greetings! Type \"/help\" for more information." << endl;
        while (1) {
            cout << "What is your name?" << endl;
            getline(cin, username);
            if (username == "/quit") throw Exception();
            if (username == "/help") {
                showHelp();
                continue;
            }
            if (username.length() <= MAX_USERNAME_LENGTH) {
                if (username.find('#') != string::npos) {
                    cout << "Symbol # is prohibited to use in names!" << endl;
                    continue;
                } else if(username.empty()) {
                    //Empty names shall not pass!
                    continue;
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

        bgThread = CreateThread(NULL, 0, BackgroundThread, (LPVOID) s, 0, NULL);
        if(bgThread == NULL) {
            cerr << "CANNOT START THREAD" << endl;
            throw Exception();
        }
        bgThreadAlive = true;
        while(bgThreadAlive) {
            /* NO SOCKET OPERATIONS HERE */
            cout << "Type name or id (with # symbol) of user you wish to chat with:" << endl;
            while (bgThreadAlive) {
                getline(cin, name);
                if (name == "/quit") throw Exception();
                if(!bgThreadAlive) break;
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
            //system("prompt [ You ]: ");
            if(peername.at(0) == '#') cout << "Public room " << peername << ":" << endl << endl;
            else cout << "Chat with " << peername << ":" << endl << endl;
            chatServer.showMessage(chatServer.getCurrentPeer());
            string str;
            while (bgThreadAlive) {
                getline(cin, str);
                if (str == "/quit") throw Exception();
                if(!bgThreadAlive) break;
                if (str == "/refresh") {
                    chatServer.showUsersList();
                    continue;
                }
                if (str == "/help") {
                    showHelp();
                    continue;
                }
                if (str == "/bye") {
                    chatServer.setCurrentPeer(0);
                    break;
                }
                {
                    SCOPE_LOCK_MUTEX(autoMutex.get());
                    MessagesToSend.push(str);
                }
                SwitchToThread();
            }
            if(!bgThreadAlive) break;
            checkOnline = true;
            checkPending = true;
            system("cls");
        }
    } catch (Exception& ex) {
        chatServer.logout();
        bgThreadAlive = false;
        WaitForSingleObject(bgThread, 60000);
    }
    cout << "Disconnecting from server..." << endl;
    shutdown(s, SD_BOTH);
    cout << "Closing socket and exiting..." << endl;
    CLOSE(s);
    EXIT(0);
}

void showHelp() {
    cout << "Command list:" << endl
         << "/refresh - show online users." << endl
         << "/bye - leave opened chat." << endl
         << "/quit - log out and exit." << endl
         << "/help - display this command list." << endl << endl;
}