/*
    C ECHO client example using sockets
    http://www.binarytides.com/server-client-example-c-sockets-linux/
*/
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <unistd.h>    //close
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr

#ifndef SERVER_IP_ADDR
#  define SERVER_IP_ADDR "192.168.1.173"
#endif

#ifndef SERVER_PORT
#  define SERVER_PORT 8888
#endif

static
int loop(void)
{
    int sock;
    struct sockaddr_in server;
    
    char message[1000] , server_reply[2000];
    
    printf("Press any key to continue...");
    getchar();
    printf("\n");
    
    //Create socket
    sock = socket(AF_INET , SOCK_DGRAM , 0);
    if (sock == -1)
    {
        perror("socket creation failed");
        return 1;
    }
    puts("Socket created\n");
     
    server.sin_addr.s_addr = inet_addr(SERVER_IP_ADDR);
    server.sin_family = AF_INET;
    server.sin_port = htons( SERVER_PORT );

    //keep communicating with server
    while(1)
    {
        struct sockaddr_in peer;
        socklen_t address_len = sizeof(server);

        printf("Enter message : ");
        scanf("%s" , message);
         
        //Send some data
        if( sendto(sock , message , strlen(message)+1 , 0, &server, address_len) < 0)
        {
            perror("Send failed");
            close(sock);
            return 1;
        }
         
        //Receive a reply from the server
        if( recvfrom(sock , server_reply , 2000 , 0, &peer, &address_len) < 0)
        {
            perror("recv failed");
            break;
        }
         
        puts("Server reply :");
        uint8_t *ipaddr = &peer.sin_addr.s_addr;
        printf("Server (%d.%d.%d.%d:%d) reply: %s\n",
                ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3],
                ntohs(peer.sin_port),
                server_reply);
    }
     
    close(sock);
    return 0;
}

int main(void)
{
    while(1)
    {
        loop();
    }
}

