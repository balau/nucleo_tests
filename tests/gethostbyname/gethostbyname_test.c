/*
    C ECHO client example using sockets
    http://www.binarytides.com/server-client-example-c-sockets-linux/
*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

static
int loop(void)
{
    struct hostent *h;
    char *name;

    printf("Press any key to continue...");
    getchar();
    printf("\n");

    name = "www.facebook.com";

    h = gethostbyname(name);
    if (h == NULL)
    {
        fprintf(stderr, "error: gethostbyname\n");
    }
    else
    {
        char **aliases;
        char **addresses;

        aliases = h->h_aliases;
        while(*aliases != NULL)
        {
            printf("alias: %s\n", *aliases);
            aliases++;
        }

        addresses = h->h_addr_list;
        while(*addresses != NULL)
        {
            struct in_addr addr;

            memcpy(&addr.s_addr, *addresses, sizeof(in_addr_t));
            printf("addr: %s\n", inet_ntoa(addr));
            addresses++;
        }

    }
    return 0;
}

int main(void)
{
    while(1)
    {
        loop();
    }
}

