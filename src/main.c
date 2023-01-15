#include <stdlib.h>
#include "client.h"

int main(int argc, char *argv[])
{
    char *ip = argv[1];
    int port = atoi(argv[2]);
    char *path = argv[3];

    int sock = createSocket();
    if (sock == -1)
        return -1;
    
    int conn = connectServer(sock, ip, port);
    if (conn == -1)
        return -1;

    char req_mesg[MAX_REQUEST_SIZE];
    int req_size = createRequestMessage(req_mesg, path, ip, port);
    if (req_size == -1)
        return -1;

    int send_size = sendRequestMessage(sock, req_mesg, req_size);
    if (send_size == -1)
        return -1;

    char res_mesg[MAX_RESPONSE_SIZE];
    int res_size = recvResponseMessage(sock, res_mesg, MAX_RESPONSE_SIZE);
    if (res_size == -1)
        return -1;

    printf("%s", res_mesg);
    closeServer(sock);
}