/*
    C ECHO client example using sockets
    http://www.binarytides.com/server-client-example-c-sockets-linux/
*/
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <unistd.h>    //close
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr

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
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        perror("socket creation failed");
        return 1;
    }
    puts("Socket created\n");
     
    server.sin_addr.s_addr = inet_addr("192.168.1.173");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed");
        close(sock);
        return 1;
    }
     
    puts("Connected\n");
     
    //keep communicating with server
    while(1)
    {
        printf("Enter message : ");
        scanf("%s" , message);
         
        //Send some data
        if( send(sock , message , strlen(message)+1 , 0) < 0)
        {
            perror("Send failed");
            close(sock);
            return 1;
        }
         
        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            perror("recv failed");
            break;
        }
         
        puts("Server reply :");
        puts(server_reply);
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

