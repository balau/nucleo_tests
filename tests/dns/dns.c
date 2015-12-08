/*
    C ECHO client example using sockets
    http://www.binarytides.com/server-client-example-c-sockets-linux/
*/
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

extern
in_addr_t getaddrinfo_dns_server;

static
int loop(void)
{
    struct addrinfo *ai;
    struct addrinfo *ai_iter;
    struct addrinfo hints;
    char *name;
    int res;

    printf("Press any key to continue...");
    getchar();
    printf("\n");

    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    name = "www.facebook.com";

    res = getaddrinfo(name, NULL, &hints, &ai);
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
    for (ai_iter = ai; ai_iter != NULL; ai_iter = ai_iter->ai_next)
    {
        struct sockaddr_in *addr;

        addr = (struct sockaddr_in *)ai_iter->ai_addr;

        printf("%s: %s\n",
                ai_iter->ai_canonname,
                inet_ntoa(addr->sin_addr));
    }

    freeaddrinfo(ai);
    return 0;
}

int main(void)
{
    while(1)
    {
        loop();
    }
}

