#include <sys/timeb.h>
#include <etcp.h>
#include <winsock2.h>

#define MINBSDSOCKERR   ( WSAEWOULDBLOCK )
#define MAXBSDSOCKERR   ( MINBSDSOCKERR + \
                        ( sizeof( bsdsocketerrs ) / \
                        sizeof( bsdsocketerrs[ 0 ] ) ) )

extern int sys_nerr;
extern char* sys_errlist[];
extern char* program_name;
static char* bsdsocketerrs[] =
        {
        "err1"
        //Всякие ошибки
        };

void init(char **argv) {
    WSADATA wsadata;
    ( program_name = strrchr(argv[0], '\\') ) ?
            program_name++ : ( program_name = argv[0] );
    WSAStartup( MAKEWORD(2,2), &wsadata );
}

/* inet_aton - версия inet_aton для SVr4 и Windows */
int inet_aton(char *cp, struct in_addr* pin) {
    int rc;

    rc = inet_addr(cp);
    if(rc == -1 && strcmp(cp, "255.255.255.255")) return 0;
    pin->s_addr = rc;
    return 1;
}

/* gettimeofday - для tselect (?)*/

/* strerror - версия, включающая код ошибок Winsock */
char* strerror(int err) {
    if(err >= 0 && err < sys_nerr)
        return sys_errlist[err];
    else if (err >= MINBSDSOCKERR && err < MAXBSDSOCKERR)
        return bsdsocketerrs[err - MINBSDSOCKERR];
    else if (err == WSASYSNOTREADY)
        return "Network subsystem is unusable";
    else if (err == WSAVERNOTSUPPORTED)
        return "This version of Winsock is not supported";
    else if (err == WSANOTINITIALISED)
        return "Winsock is not initialized";
    else
        return "Unknown error";
}