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
    string username;

    INIT();

    s = tcp_client(argv[1], argv[2]);
    ChatServer chatServer(s);

    cout << "What is your name?" << endl;
    while(1) {
        cin >> username;
        if(username.length() <= 32) {
            break;
        }
        cout << "This name is too long! Max 32 character!";
    }
    chatServer.login(username);

    int rc;
    char buf[120];

    for(;;) {
        rc = recv(s, buf, sizeof(buf), 0);
        if(rc <= 0) break;
        write(1, buf, rc);
    }

    EXIT(0);
}