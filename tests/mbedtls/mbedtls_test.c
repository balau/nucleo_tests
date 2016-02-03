
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include "mbedtls/ssl.h"
#include "mbedtls/error.h"

static
int wrap_send(void *ctx, const unsigned char *buffer, size_t len)
{
    int sock;
    int ret;

    sock = *(int *)ctx;
    ret = send(sock, buffer, len, 0);

    return ret;
}

static
int wrap_recv(void *ctx, unsigned char *buffer, size_t len)
{
    int sock;
    int ret;

    sock = *(int *)ctx;
    ret = recv(sock, buffer, len, 0);

    return ret;

}

static
int wrap_rng(void *ctx, unsigned char *buffer, size_t len)
{
    (void)ctx; /* ignore */
    while(len > 0)
    {
        *buffer++ = rand();
        len--;
    }
    return 0;
}

static
void mbedtls_printerr(int err, const char *func)
{
    char errstr[80];

    mbedtls_strerror(err, errstr, sizeof(errstr)-1);
    fprintf(stderr, "error: %s returned %d : %s", func, err, errstr );
}

extern unsigned char digicert_der[];
extern unsigned int digicert_der_len;

int main(void)
{
    struct addrinfo hints;
    struct addrinfo *ai;
    struct sockaddr_in server;
    int sock;
    int res;

    char server_name[] = "www.eff.org";
    char http_get[200] =
        "GET /index.html HTTP/1.1\r\n"
        "Host: www.eff.org\r\n"
        "\r\n";
    char http_get_resp[200];
    int cipher_list[] = { MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA, 0};

    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;

    printf("Press any key to continue...");
    getchar();
    printf("\n");

    mbedtls_ssl_init( &ssl );
    mbedtls_ssl_config_init( &conf );
    mbedtls_x509_crt_init( &cacert );

    res = mbedtls_ssl_config_defaults(
            &conf,
            MBEDTLS_SSL_IS_CLIENT,
            MBEDTLS_SSL_TRANSPORT_STREAM,
            MBEDTLS_SSL_PRESET_DEFAULT);
    if( res != 0 )
    {
        mbedtls_printerr(res, "mbedtls_ssl_config_defaults");
        return 1;
    }
#if 0
    mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_NONE);
#endif
    mbedtls_ssl_conf_ciphersuites( &conf, cipher_list);
#if 0
    res = mbedtls_ssl_conf_max_frag_len( &conf, MBEDTLS_SSL_MAX_FRAG_LEN_512);
    if( res != 0 )
    {
        mbedtls_printerr(res, "mbedtls_ssl_conf_max_frag_len");
        return 1;
    }
#endif
    mbedtls_ssl_conf_rng(&conf, wrap_rng, NULL);
    res = mbedtls_ssl_setup( &ssl, &conf);
    if( res != 0 )
    {
        mbedtls_printerr(res, "mbedtls_ssl_setup");
        return 1;
    }
    res = mbedtls_x509_crt_parse(&cacert, digicert_der, digicert_der_len);
    if( res != 0 )
    {
        mbedtls_printerr(res, "mbedtls_x509_crt_parse");
        return 1;
    }
    mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );
    res = mbedtls_ssl_set_hostname( &ssl, server_name);
    if( res != 0 )
    {
        mbedtls_printerr(res, "mbedtls_ssl_set_hostname");
        return 1;
    }

    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    res = getaddrinfo(server_name, NULL, &hints, &ai);
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
#if 1
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
    
    /* TLS connect */
    mbedtls_ssl_set_bio( &ssl, &sock, wrap_send, wrap_recv, NULL);
    res = mbedtls_ssl_handshake( &ssl );
    do
    {
        const mbedtls_x509_crt *peer_cert;

        peer_cert = mbedtls_ssl_get_peer_cert(&ssl);
        if (peer_cert == NULL)
        {
            fprintf(stderr, "no peer cert.\n");
        }
        else
        {
            int n;
            char info_str[200];

            n = mbedtls_x509_crt_info(info_str, sizeof(info_str), "", peer_cert);
            fputs("Certificate:\n", stderr);
            fputs(info_str, stderr);
            fputs("\n", stderr);
        }
    } while(0);
    if (res != 0)
    {
        mbedtls_printerr(res, "mbedtls_ssl_handshake");
        close(sock);
        return 1;
    }

    res = mbedtls_ssl_write(&ssl, (unsigned char *)http_get, strlen(http_get));
    if (res <= 0)
    {
        mbedtls_printerr(res, "mbedtls_ssl_write");
        close(sock);
        return 1;
    }
    do
    {
        res = mbedtls_ssl_read(&ssl, (unsigned char *)http_get_resp, sizeof(http_get_resp));
        if (res <= 0)
        {
            mbedtls_printerr(res, "mbedtls_ssl_read");
            close(sock);
            return 1;
        }
        fwrite(http_get_resp, res, 1, stdout);
    } while(res == sizeof(http_get_resp)); //TODO: cleaner

    /* TLS disconnect */
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    close(sock);

    return 0;
}


