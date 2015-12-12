/*
    C socket server example
    http://www.binarytides.com/server-client-example-c-sockets-linux/
*/
 
#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <errno.h>
#include <poll.h>

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
    fflush(stdout);
}

static
int loop(void)
{
    int socket_desc;
    struct sockaddr_in server;
     
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
            struct pollfd pfd;
            int ret;

            pfd.fd = socket_desc;
            pfd.events = POLLIN;

            ret = poll(&pfd, 1, 100);
            if (ret < 0)
            {
                perror("poll");
                close(socket_desc);
                return 1;
            }
            else if (ret > 0)
            {
                if (pfd.revents & POLLNVAL)
                {
                    perror("POLLNVAL");
                    close(socket_desc);
                    return 1;
                }
                if (pfd.revents & POLLIN)
                {
                    break;
                }
            }
            else
            {
                spin();
            }
        } while(1);
        //accept connection from an incoming client
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
        if (client_sock < 0 && (errno != EAGAIN))
        {
            perror("accept failed.");
            close(socket_desc);
            return 1;
        }
        puts("\rConnection accepted");

        while(1)
        {
            struct pollfd pfd;
            int ret;

            pfd.fd = client_sock;
            pfd.events = POLLIN;

            ret = poll(&pfd, 1, 100);
            if ((pfd.revents & POLLNVAL) || (ret == -1))
            {
                perror("recv failed");
                close(socket_desc);
                close(client_sock);
                return 1;
            }
            else if (pfd.revents & (POLLHUP|POLLERR))
            {
                puts("Client disconnected");
                fflush(stdout);
                close(client_sock);
                break;
            }
            else if (pfd.revents & POLLIN)
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

