#ifdef __cplusplus
extern "C" {
#endif
char* program_name;
#ifdef __cplusplus
}
#endif

#include <iostream>
#include "etcp.h"
#include "ChatServer.h"

using namespace std;

int main(int argc, char** argv) {
    struct sockaddr_in peer;
    SOCKET s;
    string username, peername, name;

    INIT();

    s = tcp_client(argv[1], argv[2]);
    ChatServer chatServer(s);

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

        cout << "Type name or id (with # symbol) of user you wish to chat with:" << endl;
        while (1) {
            cin >> name;
            if (name == "/quit") throw Exception();
            if (name == "/refresh") { chatServer.showUsersList(); continue; }
            if (name.length() <= MAX_USERNAME_LENGTH) {
                peername = chatServer.startChat(name);
                if(!peername.empty()) {
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
        system("cls");
        cout << "Chat with " << peername << ":" << endl << endl;

    } catch (Exception& ex) {
        cout << "Disconnecting from server..." << endl;
        // Quit gracefully.
    }
    cout << "Closing socket and exiting..." << endl;
    closesocket(s);
    EXIT(0);
}