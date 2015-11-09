/*
    C socket server example
    http://www.binarytides.com/server-client-example-c-sockets-linux/
*/
 
#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write

static
int loop(void)
{
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
     
    printf("Press any key to continue...");
    getchar();
    printf("\n");
    
    //Create socket
    socket_desc = socket(AF_INET , SOCK_DGRAM , 0);
    if (socket_desc == -1)
    {
        perror("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) != 0)
    {
        //print the error message
        perror("bind failed.");
        close(socket_desc);
        return 1;
    }
    puts("bind done");
    
    while(1)
    {
       struct sockaddr_in peer;
       socklen_t address_len = sizeof(peer);

        //Receive a message from client
        while( (read_size = recvfrom(socket_desc , client_message , 2000 , 0, &peer, &address_len)) > 0 )
        {
            uint8_t *ipaddr = &peer.sin_addr.s_addr;
            printf("recv(%d.%d.%d.%d:%d): %s\n",
                    ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3],
                    ntohs(peer.sin_port),
                    client_message);
            //Send the message back to client
            sendto(socket_desc , client_message , strlen(client_message)+1, 0, &peer, address_len);
        }

        if(read_size == -1)
        {
            perror("recv failed");
            close(socket_desc);
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

