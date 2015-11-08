/*
 * Trying different socket errors.
 */
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <unistd.h>    //close
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <assert.h>
#include <errno.h>

#ifndef SERVER_IP_ADDR
#  define SERVER_IP_ADDR "192.168.1.173"
#endif

#ifndef SERVER_PORT
#  define SERVER_PORT 8888
#endif

#define assert_equal(x, y) do { \
        if (x != y) { fprintf(stderr, "%d != %d\n", x, y); } \
        assert(x == y); \
    } while(0);

int main(void)
{
    int sock;
    int ret;
    struct sockaddr_in server;

    sock = socket(AF_INET , SOCK_STREAM , 1);
    assert_equal(sock, -1);
    assert_equal(errno, EPROTONOSUPPORT);

    errno = 0;
    sock = socket(0xFFFF , SOCK_STREAM , 0);
    assert_equal(sock, -1);
    assert_equal(errno, EAFNOSUPPORT);
    
    errno = 0;
    sock = socket(AF_INET , 0xFFFF , 0);
    assert_equal(sock, -1);
    assert_equal(errno, EINVAL);

    errno = 0;
    sock = socket(AF_INET , SOCK_STREAM , 0);
    assert(sock != -1);
    assert_equal(errno, 0);
    
    server.sin_addr.s_addr = inet_addr(SERVER_IP_ADDR);
    server.sin_family = AF_INET;
    server.sin_port = htons( SERVER_PORT );

    errno = 0;
    ret = connect(0xFFFF, (struct sockaddr *)&server, sizeof(server));
    assert_equal(ret, -1);
    assert_equal(errno, EBADF);
    
    errno = 0;
    ret = connect(STDIN_FILENO, (struct sockaddr *)&server, sizeof(server));
    assert_equal(ret, -1);
    assert_equal(errno, ENOTSOCK);

    server.sin_family = 0xFFFF;
    errno = 0;
    ret = connect(sock, (struct sockaddr *)&server, sizeof(server));
    assert_equal(ret, -1);
    assert_equal(errno, EAFNOSUPPORT);
    
    server.sin_addr.s_addr = inet_addr("0.0.0.0");
    server.sin_family = AF_INET;
    server.sin_port = htons( SERVER_PORT );
    errno = 0;
    ret = connect(sock, (struct sockaddr *)&server, sizeof(server));
    assert_equal(ret, -1);
    assert_equal(errno, ECONNREFUSED);

    errno = 0;
    ret = close(0xFFFF);
    assert_equal(ret, -1);
    assert_equal(errno, EBADF);

    errno = 0;
    ret = close(sock);
    assert_equal(ret, 0);
    assert_equal(errno, 0);
    
    errno = 0;
    ret = close(sock);
    assert_equal(ret, -1);
    assert_equal(errno, EBADF);

    server.sin_addr.s_addr = inet_addr(SERVER_IP_ADDR);
    server.sin_family = AF_INET;
    server.sin_port = htons( SERVER_PORT );
    errno = 0;
    ret = connect(sock, (struct sockaddr *)&server, sizeof(server));
    assert_equal(ret, -1);
    assert_equal(errno, EBADF);

    puts("All tests OK");
    return 0;
}

