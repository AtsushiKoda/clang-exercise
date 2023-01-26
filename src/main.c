#include <stdlib.h>
#include "client.h"

int main(int argc, char *argv[])
{
    char *ip = argv[1];
    int port = atoi(argv[2]);
    char *path = argv[3];

    char req_mesg[MAX_REQUEST_SIZE];
    char res_mesg[MAX_RESPONSE_SIZE];
    
    return do_webclient(req_mesg, res_mesg, ip, port, path);
}