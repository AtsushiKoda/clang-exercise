#include <stdio.h>
#include <string.h>
#include <sys/socket.h> // socket(), connect()
#include <arpa/inet.h>  // struct sockaddr_in

#define MAX_SIZE 128
#define MAX_REQUEST_SIZE 1024  // とりあえず1KB
#define MAX_RESPONSE_SIZE 4096 // とりあえず4KB

int createSocket();
int connectServer(int sock, char *ip, int port);
int createRequestMessage(char *req_mesg, char *path, char *ip, int port);
int sendRequestMessage(int sock, char *req_mesg, int req_size);
int recvResponseMessage(int sock, char *res_mesg, int buf_size);
int closeServer(int sock);
