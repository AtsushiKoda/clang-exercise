#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "../src/client.c"

int socketMock(int __domain, int __type, int __protocol)
{
    return mock()
    .actualCall("socket")
    .withIntParameter("domain", __domain)
    .withIntParameter("type", __type)
    .withIntParameter("protocol", __protocol)
    .returnIntValue();
}

int connectMock(int __fd, const sockaddr *__addr, socklen_t __len)
{
    struct sockaddr_in *addr = (struct sockaddr_in *)__addr;
    return mock()
    .actualCall("connect")
    .withIntParameter("sock", __fd)
    .withIntParameter("addr.sin_family", addr->sin_family)
    .withIntParameter("addr.sin_addr.s_addr", addr->sin_addr.s_addr)
    .withIntParameter("addr.sin_port", addr->sin_port)
    .withIntParameter("len", __len)
    .returnIntValue();
}

ssize_t sendMock(int __fd, const void *__buf, size_t __n, int __flags)
{
    return mock()
    .actualCall("send")
    .withIntParameter("sock", __fd)
    .withParameter("req_mesg", __buf)
    .withIntParameter("size", __n)
    .withIntParameter("flags", __flags)
    .returnIntValue();
}

ssize_t recvMock(int __fd, void *__buf, size_t __n, int __flags)
{
    return mock()
    .actualCall("recv")
    .withIntParameter("sock", __fd)
    .withParameter("mesg", __buf)
    .withIntParameter("size", __n)
    .withIntParameter("flags", __flags)
    .returnLongIntValue();
}

int closeMock(int __fd)
{
    return mock()
    .actualCall("close")
    .withIntParameter("sock", __fd)
    .returnIntValue();
}

TEST_GROUP(testWebClient){
    void setup(){
        UT_PTR_SET(client->socket, socketMock);
        UT_PTR_SET(client->connect, connectMock);
        UT_PTR_SET(client->send, sendMock);
        UT_PTR_SET(client->recv, recvMock);
        UT_PTR_SET(client->close, closeMock);
    }

    void teardown()
    {
        mock().clear();
    }
};

TEST(testWebClient, createSocketSuccess)
{
    mock()
    .expectOneCall("socket")
    .withIntParameter("domain", PF_INET)
    .withIntParameter("type", SOCK_STREAM)
    .withIntParameter("protocol", 0)
    .andReturnValue(1);

    int sock = createSocket();
    CHECK_EQUAL(1, sock);
    mock().checkExpectations();
}

TEST(testWebClient, createSocketFail)
{
    mock()
    .expectOneCall("socket")
    .withIntParameter("domain", PF_INET)
    .withIntParameter("type", SOCK_STREAM)
    .withIntParameter("protocol", 0)
    .andReturnValue(-1);

    int sock = createSocket();
    CHECK_EQUAL(-1, sock);
    mock().checkExpectations();
}

TEST(testWebClient, connectServerSuccess)
{
    int sock = 1;
    char *ip = "0.0.0.0";
    int port = 8080;

    struct sockaddr_in addr;

    mock()
    .expectOneCall("connect")
    .withIntParameter("sock", sock)
    .withIntParameter("addr.sin_family", AF_INET)
    .withIntParameter("addr.sin_addr.s_addr", inet_addr(ip))
    .withIntParameter("addr.sin_port", htons(port))
    .withIntParameter("len", sizeof(addr))
    .andReturnValue(1);

    int conn = connectServer(sock, ip, port);
    CHECK_EQUAL(1, conn);
    mock().checkExpectations();
}

TEST(testWebClient, connectServerFail)
{
    int sock = 1;
    char *ip = "0.0.0.0";
    int port = 8080;

    struct sockaddr_in addr;

    mock()
    .expectOneCall("connect")
    .withIntParameter("sock", sock)
    .withIntParameter("addr.sin_family", AF_INET)
    .withIntParameter("addr.sin_addr.s_addr", inet_addr(ip))
    .withIntParameter("addr.sin_port", htons(port))
    .withIntParameter("len", sizeof(addr))
    .andReturnValue(-1);

    int conn = connectServer(sock, ip, port);
    CHECK_EQUAL(-1, conn);
    mock().checkExpectations();
}

TEST(testWebClient, createReqMesgSuccess)
{
    char req_mesg[MAX_REQUEST_SIZE];
    char *path = "/";
    char *ip = "0.0.0.0";
    int port = 8080;
    int req_size = createRequestMessage(req_mesg, path, ip, port);
    CHECK_EQUAL(60, req_size);
}

TEST(testWebClient, sendReqMesgSuccess)
{
    int sock = 1;
    char *req_mesg = "GET / HTTP/1.1\nUser-Agent: My Webclient\nHost: 0.0.0.0:8080\n\n";
    int req_size = 60;

    mock()
    .expectOneCall("send")
    .withIntParameter("sock", sock)
    .withParameter("req_mesg", (const void *)req_mesg)
    .withIntParameter("size", req_size)
    .withIntParameter("flags", 0)
    .andReturnValue(60);

    int send_size = sendRequestMessage(sock, req_mesg, req_size);

    CHECK_EQUAL(60, send_size);
    mock().checkExpectations();
}

TEST(testWebClient, sendReqMesgFail)
{
    int sock = 1;
    char *req_mesg = "GET / HTTP/1.1\nUser-Agent: My Webclient\nHost: 0.0.0.0:8080\n\n";
    int req_size = 60;

    mock()
    .expectOneCall("send")
    .withIntParameter("sock", sock)
    .withParameter("req_mesg", (const void *)req_mesg)
    .withIntParameter("size", req_size)
    .withIntParameter("flags", 0)
    .andReturnValue(-1);

    int send_size = sendRequestMessage(sock, req_mesg, req_size);

    CHECK_EQUAL(-1, send_size);
    mock().checkExpectations();
}

TEST(testWebClient, recvResMesgSuccess)
{
    int sock = 1;
    char res_mesg[MAX_RESPONSE_SIZE];
    ssize_t size = (ssize_t)sizeof(res_mesg);
    int total_recv_size = 0;

    mock()
    .expectOneCall("recv")
    .withIntParameter("sock", sock)
    .withParameter("mesg", (void *)&res_mesg[total_recv_size])
    .withIntParameter("size", MAX_RESPONSE_SIZE)
    .withIntParameter("flags", 0)
    .andReturnValue(size);
    // 2回目呼び出し。接続が正しく終了。
    mock()
    .expectOneCall("recv")
    .ignoreOtherParameters()
    .andReturnValue(0);

    int res_size = recvResponseMessage(sock, res_mesg, MAX_RESPONSE_SIZE);

    total_recv_size += size;
    CHECK_EQUAL(total_recv_size, res_size);
    mock().checkExpectations();
}

TEST(testWebClient, recvResMesgFail)
{
    int sock = 1;
    char res_mesg[MAX_RESPONSE_SIZE];
    int total_recv_size = 0;

    mock()
    .expectOneCall("recv")
    .withIntParameter("sock", sock)
    .withParameter("mesg", (void *)&res_mesg[total_recv_size])
    .withIntParameter("size", MAX_RESPONSE_SIZE)
    .withIntParameter("flags", 0)
    .andReturnValue(-1);
    
    int res_size = recvResponseMessage(sock, res_mesg, MAX_RESPONSE_SIZE);

    CHECK_EQUAL(-1, res_size);
    mock().checkExpectations();
}

TEST(testWebClient, closeServerSuccess)
{
    int sock = 1;

    mock()
    .expectOneCall("close")
    .withIntParameter("sock", sock)
    .andReturnValue(1);

    int st = closeServer(sock);
    CHECK_EQUAL(0, st);
    mock().checkExpectations();
}

TEST(testWebClient, closeServerFail)
{
    int sock = 1;

    mock()
    .expectOneCall("close")
    .withIntParameter("sock", sock)
    .andReturnValue(-1);

    int st = closeServer(sock);
    CHECK_EQUAL(-1, st);
    mock().checkExpectations();
}

TEST(testWebClient, doWebClientSocketFail)
{
    char *ip = "0.0.0.0";
    int port = 8080;
    char *path = "/";
    char req_mesg[MAX_REQUEST_SIZE];
    char res_mesg[MAX_RESPONSE_SIZE];

    mock()
    .expectOneCall("socket")
    .withIntParameter("domain", PF_INET)
    .withIntParameter("type", SOCK_STREAM)
    .withIntParameter("protocol", 0)
    .andReturnValue(-1);

    int status = do_webclient(req_mesg, res_mesg, ip, port, path);
    CHECK_EQUAL(SOCKET_CONNECTION_ERROR, status);
    mock().checkExpectations();
}

TEST(testWebClient, doWebClientServerConnFail)
{
    char *ip = "0.0.0.0";
    int port = 8080;
    char *path = "/";
    char req_mesg[MAX_REQUEST_SIZE];
    char res_mesg[MAX_RESPONSE_SIZE];

    mock()
    .expectOneCall("socket")
    .withIntParameter("domain", PF_INET)
    .withIntParameter("type", SOCK_STREAM)
    .withIntParameter("protocol", 0)
    .andReturnValue(1);

    struct sockaddr_in addr;
    int sock = 1;
    mock()
    .expectOneCall("connect")
    .withIntParameter("sock", sock)
    .withIntParameter("addr.sin_family", AF_INET)
    .withIntParameter("addr.sin_addr.s_addr", inet_addr(ip))
    .withIntParameter("addr.sin_port", htons(port))
    .withIntParameter("len", sizeof(addr))
    .andReturnValue(-1);

    int status = do_webclient(req_mesg, res_mesg, ip, port, path);
    CHECK_EQUAL(SERVER_CONNECTION_ERROR, status);
    mock().checkExpectations();
}

TEST(testWebClient, doWebClientSendReqMesgFail)
{
    char *ip = "0.0.0.0";
    int port = 8080;
    char *path = "/";
    char req_mesg[MAX_REQUEST_SIZE];
    char res_mesg[MAX_RESPONSE_SIZE];

    mock()
    .expectOneCall("socket")
    .withIntParameter("domain", PF_INET)
    .withIntParameter("type", SOCK_STREAM)
    .withIntParameter("protocol", 0)
    .andReturnValue(1);

    struct sockaddr_in addr;
    int sock = 1;
    mock()
    .expectOneCall("connect")
    .withIntParameter("sock", sock)
    .withIntParameter("addr.sin_family", AF_INET)
    .withIntParameter("addr.sin_addr.s_addr", inet_addr(ip))
    .withIntParameter("addr.sin_port", htons(port))
    .withIntParameter("len", sizeof(addr))
    .andReturnValue(1);

    int req_size = 60;

    mock()
    .expectOneCall("send")
    .withIntParameter("sock", sock)
    .withParameter("req_mesg", (const void *)req_mesg)
    .withIntParameter("size", req_size)
    .withIntParameter("flags", 0)
    .andReturnValue(-1);
   
    int status = do_webclient(req_mesg, res_mesg, ip, port, path);
    CHECK_EQUAL(SEND_MESSAGE_ERROR, status);
    mock().checkExpectations();
}

TEST(testWebClient, doWebClientCloseFail)
{
    char *ip = "0.0.0.0";
    int port = 8080;
    char *path = "/";
    char req_mesg[MAX_REQUEST_SIZE];
    char res_mesg[MAX_RESPONSE_SIZE];

    mock()
    .expectOneCall("socket")
    .withIntParameter("domain", PF_INET)
    .withIntParameter("type", SOCK_STREAM)
    .withIntParameter("protocol", 0)
    .andReturnValue(1);

    struct sockaddr_in addr;
    int sock = 1;
    mock()
    .expectOneCall("connect")
    .withIntParameter("sock", sock)
    .withIntParameter("addr.sin_family", AF_INET)
    .withIntParameter("addr.sin_addr.s_addr", inet_addr(ip))
    .withIntParameter("addr.sin_port", htons(port))
    .withIntParameter("len", sizeof(addr))
    .andReturnValue(1);

    int req_size = 60;

    mock()
    .expectOneCall("send")
    .withIntParameter("sock", sock)
    .withParameter("req_mesg", (const void *)req_mesg)
    .withIntParameter("size", req_size)
    .withIntParameter("flags", 0)
    .andReturnValue(60);
   
    int total_recv_size = 0;

    mock()
    .expectOneCall("recv")
    .withIntParameter("sock", sock)
    .withParameter("mesg", (void *)&res_mesg[total_recv_size])
    .withIntParameter("size", MAX_RESPONSE_SIZE)
    .withIntParameter("flags", 0)
    .andReturnValue(-1);
    
    int status = do_webclient(req_mesg, res_mesg, ip, port, path);
    CHECK_EQUAL(RECEIVE_MESSAGE_ERROR, status);
    mock().checkExpectations();
}

TEST(testWebClient, doWebClientRecvResMesgFail)
{
    char *ip = "0.0.0.0";
    int port = 8080;
    char *path = "/";
    char req_mesg[MAX_REQUEST_SIZE];
    char res_mesg[MAX_RESPONSE_SIZE];

    mock()
    .expectOneCall("socket")
    .withIntParameter("domain", PF_INET)
    .withIntParameter("type", SOCK_STREAM)
    .withIntParameter("protocol", 0)
    .andReturnValue(1);

    struct sockaddr_in addr;
    int sock = 1;
    mock()
    .expectOneCall("connect")
    .withIntParameter("sock", sock)
    .withIntParameter("addr.sin_family", AF_INET)
    .withIntParameter("addr.sin_addr.s_addr", inet_addr(ip))
    .withIntParameter("addr.sin_port", htons(port))
    .withIntParameter("len", sizeof(addr))
    .andReturnValue(1);

    int req_size = 60;

    mock()
    .expectOneCall("send")
    .withIntParameter("sock", sock)
    .withParameter("req_mesg", (const void *)req_mesg)
    .withIntParameter("size", req_size)
    .withIntParameter("flags", 0)
    .andReturnValue(60);
   
    int total_recv_size = 0;
    ssize_t size = (ssize_t)sizeof(res_mesg);

    mock()
    .expectOneCall("recv")
    .withIntParameter("sock", sock)
    .withParameter("mesg", (void *)&res_mesg[total_recv_size])
    .withIntParameter("size", MAX_RESPONSE_SIZE)
    .withIntParameter("flags", 0)
    .andReturnValue(size);

    mock()
    .expectOneCall("recv")
    .ignoreOtherParameters()
    .andReturnValue(0);

    mock()
    .expectOneCall("close")
    .withIntParameter("sock", sock)
    .andReturnValue(-1);

    int status = do_webclient(req_mesg, res_mesg, ip, port, path);
    CHECK_EQUAL(CLOSE_ERROR, status);
    mock().checkExpectations();
}

TEST(testWebClient, doWebClientSuccess)
{
    char *ip = "0.0.0.0";
    int port = 8080;
    char *path = "/";
    char req_mesg[MAX_REQUEST_SIZE];
    char res_mesg[MAX_RESPONSE_SIZE];

    mock()
    .expectOneCall("socket")
    .withIntParameter("domain", PF_INET)
    .withIntParameter("type", SOCK_STREAM)
    .withIntParameter("protocol", 0)
    .andReturnValue(1);

    struct sockaddr_in addr;
    int sock = 1;
    mock()
    .expectOneCall("connect")
    .withIntParameter("sock", sock)
    .withIntParameter("addr.sin_family", AF_INET)
    .withIntParameter("addr.sin_addr.s_addr", inet_addr(ip))
    .withIntParameter("addr.sin_port", htons(port))
    .withIntParameter("len", sizeof(addr))
    .andReturnValue(1);

    int req_size = 60;

    mock()
    .expectOneCall("send")
    .withIntParameter("sock", sock)
    .withParameter("req_mesg", (const void *)req_mesg)
    .withIntParameter("size", req_size)
    .withIntParameter("flags", 0)
    .andReturnValue(60);
   
    int total_recv_size = 0;
    ssize_t size = (ssize_t)sizeof(res_mesg);

    mock()
    .expectOneCall("recv")
    .withIntParameter("sock", sock)
    .withParameter("mesg", (void *)&res_mesg[total_recv_size])
    .withIntParameter("size", MAX_RESPONSE_SIZE)
    .withIntParameter("flags", 0)
    .andReturnValue(size);

    mock()
    .expectOneCall("recv")
    .ignoreOtherParameters()
    .andReturnValue(0);

    mock()
    .expectOneCall("close")
    .withIntParameter("sock", sock)
    .andReturnValue(0);

    int status = do_webclient(req_mesg, res_mesg, ip, port, path);
    CHECK_EQUAL(CLIENT_SUCCESS, status);
    mock().checkExpectations();
}

int main(int argc, char **argv)
{
    return CommandLineTestRunner::RunAllTests(argc, argv);
}