#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> // socket(), connect()
#include <arpa/inet.h>  // struct sockaddr_in

#define MAX_SIZE 128
#define MAX_REQUEST_SIZE 1024  // とりあえず1KB
#define MAX_RESPONSE_SIZE 4096 // とりあえず4KB

// CppUMockのために関数ポインタをフィールドに持つ構造体を定義
struct socket_api
{
    int (*socket)(int __domain, int __type, int __protocol);
    int (*connect)(int __fd, const struct sockaddr *__addr, socklen_t __len);
    ssize_t (*send)(int __fd, const void *__buf, size_t __n, int __flags);
    ssize_t (*recv)(int __fd, void *__buf, size_t __n, int __flags);
    int (*close)(int __fd);
};

int createSocket();
int connectServer(int sock, char *ip, int port);
int createRequestMessage(char *req_mesg, char *path, char *ip, int port);
int sendRequestMessage(int sock, char *req_mesg, int req_size);
int recvResponseMessage(int sock, char *res_mesg, int buf_size);
int closeServer(int sock);
