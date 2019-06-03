#ifndef SERVER_H
#define SERVER_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

int Link(){
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    char buf[1000];

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6000);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);
    
    int len = sizeof(struct sockaddr_in);

}

#endif