#ifdef __cplusplus
extern "C" {
#endif
char* program_name;
#ifdef __cplusplus
}
#endif

#include <iostream>
#include "etcp.h"

using namespace std;

int main(int argc, char** argv) {
    struct sockaddr_in peer;
    SOCKET s;

    INIT();

    s = tcp_client(argv[1], argv[2]);

    int rc;
    char buf[120];

    for(;;) {
        rc = recv(s, buf, sizeof(buf), 0);
        if(rc <= 0) break;
        write(1, buf, rc);
    }

    EXIT(0);
}