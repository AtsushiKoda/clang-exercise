#include "client.h"

struct socket_api socket_api = {
    .socket = socket,
    .connect = connect,
    .send = send,
    .recv = recv,
    .close = close,
};

struct socket_api *client = &socket_api;

/*
メモ:
> socket()にPF_*、bind()にAF_*を使うべきらしい[^2]
> [^2]: ショーン・ウォルトン著 野村純子訳 「Linuxソケットプログラミング」より
https://qiita.com/progrunner/items/bdc37d407514addde1f9

PF_INETはprotocol family internetの略
*/
int createSocket()
{
    int sock;
    sock = client->socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return -1;

    return sock;
}

int connectServer(int sock, char *ip, int port)
{
    int conn;
    /*
    メモ:
    > connect() システムコールのマニュアルには、sockaddr構造体を使うようにと
    書かれているが、TCP/IP では、そのサブクラス(オブジェクト指向用語)である
    sockaddr_in を使う。
    https://www.coins.tsukuba.ac.jp/~syspro/2022/2022-06-29/sockaddr.html
    */
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    // ネットワークバイトオーダーに変換 (ビッグエンディアン)
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    // 第二引数を`sockaddr`型にキャストする
    conn = client->connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (conn < 0)
        return -1;

    return conn;
}

int createRequestMessage(char *req_mesg, char *path, char *ip, int port)
{
    /*
    リクエストメッセージはリクエストライン、ヘッダーフィールド、ボディの3つで構成されている。
    - リクエストライン: クライアントがサーバに対して要求することを示す
        - メソッド(GET, POSTなど)、リクエストURI、HTTPバージョンの3つで構成
    - リクエストヘッダー: リクエストメッセージを聖y号するためのヘッダー
        - Hostヘッダー: webブラウザがリクエストするwebサーバのドメイン名とポート番号がセット。
            HTTP/1.1で唯一必須。
        - User-Agentヘッダー: webブラウザやOSなど、ユーザの環境を表す。
    */
    char req_line[MAX_SIZE];
    char req_header[MAX_SIZE];

    sprintf(req_line, "GET %s HTTP/1.1", path);
    sprintf(req_header, "User-Agent: My Webclient\nHost: %s:%d", ip, port);
    sprintf(req_mesg, "%s\n%s\n\n", req_line, req_header);

    int req_size = strlen(req_mesg);

    if (req_size <= 0)
        return -1;

    return req_size;
}

int sendRequestMessage(int sock, char *req_mesg, int req_size)
{
    int send_size;

    // send()の第4引数のflagは送信時の動作の詳細設定を行う。
    send_size = client->send(sock, req_mesg, req_size, 0);

    if (send_size == -1)
        return -1;

    return send_size;
}

int recvResponseMessage(int sock, char *res_mesg, int buf_size)
{
    int total_recv_size = 0;

    while (1)
    {
        int recv_size = client->recv(sock, &res_mesg[total_recv_size], buf_size, 0);
        if (recv_size == -1)
            return -1;

        if (recv_size == 0)
            break;

        total_recv_size += recv_size;
    }

    return total_recv_size;
}

int closeServer(int sock)
{
    if (client->close(sock) == -1)
        return -1;
    else
        return 0;
}

int do_webclient(char *req_mesg, char *res_mesg, char *ip, int port, char *path)
{
    int sock = createSocket();
    if (sock == -1)
    {
        fprintf(stderr, "connect error\n");
        return SOCKET_CONNECTION_ERROR;
    }

    int conn = connectServer(sock, ip, port);
    if (conn == -1)
    {
        fprintf(stderr, "server connection error\n");
        return SERVER_CONNECTION_ERROR;
    }

    int req_size = createRequestMessage(req_mesg, path, ip, port);
    if (req_size == -1)
    {
        fprintf(stderr, "request message error\n");
        return REQUEST_MESSAGE_ERROR;
    }

    int send_size = sendRequestMessage(sock, req_mesg, req_size);
    if (send_size == -1)
    {
        fprintf(stderr, "send error\n");
        return SEND_MESSAGE_ERROR;
    }
    
    int res_size = recvResponseMessage(sock, res_mesg, MAX_RESPONSE_SIZE);
    if (res_size == -1)
    {
        fprintf(stderr, "recv error\n");
        return RECEIVE_MESSAGE_ERROR;
    }

    fprintf(stdout, "%s", res_mesg);
    if (closeServer(sock) == -1)
    {
        fprintf(stderr, "close error\n");
        return CLOSE_ERROR;
    }
    else
        return CLIENT_SUCCESS;
}
