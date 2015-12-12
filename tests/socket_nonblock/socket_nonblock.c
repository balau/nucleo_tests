/*
    C socket server example
    http://www.binarytides.com/server-client-example-c-sockets-linux/
*/
 
#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <fcntl.h>
#include <errno.h>

static
void spin(void)
{
    static int i_spin = 0;
    const char spin[4] = "-/|\\";

    printf("%c", spin[i_spin % sizeof(spin)]);
    i_spin++;
    if ((i_spin % 41) == 0)
    {
        printf("\r");
    }
}

static
int loop(void)
{
    int socket_desc;
    struct sockaddr_in server;
    int ret;
     
    printf("Press any key to continue...");
    getchar();
    printf("\n");
    
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        perror("Could not create socket");
        return 1;
    }
    puts("Socket created");

    ret = fcntl(socket_desc, F_SETFL, O_NONBLOCK);
    if (ret == -1)
    {
        perror("fcntl(*, F_SETFL, O_NONBLOCK)");
        close(socket_desc);
        return 1;
    }
    printf("status: %08x\n", fcntl(socket_desc, F_GETFL));
     
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
     
    //Listen
    if (listen(socket_desc , SOMAXCONN) != 0)
    {
        //print the error message
        perror("listen failed.");
        close(socket_desc);
        return 1;
    }
    
    while(1)
    { 
        int client_sock, c;
        struct sockaddr_in client;

        //Accept and incoming connection
        puts("Waiting for incoming connections...");
        c = sizeof(struct sockaddr_in);
        do
        {
            //accept connection from an incoming client
            client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
            if (client_sock < 0 && (errno != EAGAIN))
            {
                perror("accept failed.");
                close(socket_desc);
                return 1;
            }
            else
            {
                spin();
            }
        } while(client_sock <= 0);
        puts("\rConnection accepted");

        ret = fcntl(client_sock, F_SETFL, O_NONBLOCK);
        if (ret == -1)
        {
            perror("fcntl(*, F_SETFL, O_NONBLOCK)");
            close(socket_desc);
            close(client_sock);
            return 1;
        }
        
        while(1)
        {
            char client_message[256];
            ssize_t read_size;

            read_size = recv(client_sock , client_message , sizeof(client_message), 0);
            if (read_size > 0)
            {
                client_message[read_size] = '\0'; /* NULL-terminate before printing */
                printf("\rrecv: %s\n", client_message);
                //Send the message back to client
                write(client_sock , client_message , read_size);
            }
            else if(read_size == 0)
            {
                puts("Client disconnected");
                fflush(stdout);
                close(client_sock);
                break;
            }
            else if((read_size == -1) && (errno != EAGAIN))
            {
                perror("recv failed");
                close(socket_desc);
                close(client_sock);
                return 1;
            }
            else
            {
                spin();
            }
        }
    }
    close(socket_desc);
     
    return 0;
}

int main(void)
{
    while(1)
    {
        loop();
    }
}

