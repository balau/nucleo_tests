
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <wolfssl/ssl.h>

static
void ssl_print_error(WOLFSSL * ssl, int ret)
{
    int err;
    char err_str[80];

    err = wolfSSL_get_error(ssl, ret);
    wolfSSL_ERR_error_string(err, err_str);
    fprintf(stderr, "err = %d, %s\n", err, err_str);
}

int main(void)
{
    struct addrinfo hints;
    struct addrinfo *ai;
    struct sockaddr_in server;
    int sock;
    int res;
    WOLFSSL_CTX* ctx;
    WOLFSSL*     ssl;
    char http_get[200] =
        "GET /index.html HTTP/1.1\r\n"
        "Host: www.eff.org\r\n"
        "\r\n";
    char http_get_resp[200];

    printf("Press any key to continue...");
    getchar();
    printf("\n");

    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    res = getaddrinfo("www.eff.org", NULL, &hints, &ai);
    if (res != 0)
    {
        if (res == EAI_SYSTEM)
        {
            perror("getaddrinfo");
        }
        else
        {
            fprintf(stderr, "error: getaddrinfo: %d\n", res);
        }
        return 1;
    }
    if (ai == NULL)
    {
        fprintf(stderr, "error: getaddrinfo : output is NULL\n");
        return 1;
    }

    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        perror("socket creation failed");
        return 1;
    }
#if 0
    server = *((const struct sockaddr_in *)ai->ai_addr);
    server.sin_port = htons( 443 ); /* HTTPS */
#else
    /* 
     * nslookup www.eff.org -> 69.50.225.155
     * socat TCP-LISTEN:44333 TCP:69.50.225.155:443
     */
    server.sin_family = AF_INET;
    server.sin_port = htons(44333);
    server.sin_addr.s_addr = inet_addr("192.168.1.173"); /* my PC */
#endif
    freeaddrinfo(ai);
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed");
        close(sock);
        return 1;
    }
    wolfSSL_Init();
    if ((ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method())) == NULL) {
        fprintf(stderr, "SSL_CTX_new error.\n");
        close(sock);
        return 1;
    }
    wolfSSL_SetIOSend(ctx, EmbedSend);
    wolfSSL_SetIORecv(ctx, EmbedReceive);
    ssl = wolfSSL_new(ctx);
    if (ssl == NULL)
    {
        fprintf(stderr, "wolfSSL_new error.\n");
        close(sock);
        return 1;
    }
    wolfSSL_set_verify(ssl, SSL_VERIFY_NONE, NULL);
    wolfSSL_set_fd(ssl, sock);

    res = wolfSSL_connect(ssl);
    if (res != SSL_SUCCESS)
    {
        fprintf(stderr, "wolfSSL_connect error.\n");
        ssl_print_error(ssl, res);
        close(sock);
        return 1;
    }
    
    res = wolfSSL_write(ssl, http_get, strlen(http_get));
    if (res <= 0)
    {
        fprintf(stderr, "wolfSSL_write error.\n");
        ssl_print_error(ssl, res);
        close(sock);
        return 1;
    }
    do
    {
        res = wolfSSL_read(ssl, http_get_resp, sizeof(http_get_resp));
        if (res <= 0)
        {
            fprintf(stderr, "wolfSSL_read error.\n");
            ssl_print_error(ssl, res);
            close(sock);
            return 1;
        }
        fwrite(http_get_resp, res, 1, stdout);
    } while(res == sizeof(http_get_resp)); //TODO: cleaner

    wolfSSL_free(ssl);
    wolfSSL_CTX_free(ctx);
    wolfSSL_Cleanup();

    close(sock);

    return 0;
}


