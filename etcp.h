#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "skel.h"

#define TRUE    1
#define FALSE   0
#define NLISTEN 5   // Максимальное число ожидающих соединений
#define NSMB    5   // Число буферов в разделяемой памяти
#define SMBUFSZ 256 // Размер буфера в разделяемой памяти

#ifdef __SVR4
#define bzero(b,n) memset( ( b ), 0, ( n ) )
#endif

typedef void ( *tofunc_t )(void *);

void error(int, int, const char *, ...);
int readn(SOCKET, char *, size_t);
int readvrec(SOCKET, char *, size_t);
//int readcrlf(SOCKET, char*, size_t);
//int readline(SOCKET, char*, size_t);
SOCKET tcp_server(char *, char *);
SOCKET tcp_client(char *, char *);
//int udp_server(char*, char*);
//int udp_client(char*, char*, struct sockaddr_in*);
//int tselect(int, fd_set*, fd_set*, fd_set*);
//unsigned int timeout(tofunc_t, void*, int);
//void untimeout(unsigned int);
//void init_smb(int);
//void *smballoc(void);
//void smbfree(void*);
//void smbsend(SOCKET, void*);
//void* smbrecv(SOCKET);
void set_address(char *, char *, struct sockaddr_in *, char *);
int inet_aton(char *cp, struct in_addr* pin);

#ifdef __cplusplus
}
#endif