/*
 * Name: Evan Chou
 * Title: Lab4 TCP server
 * Date: October 16, 2020
 * Description: This program runs the TCP server for TCP client-server distribution
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#define N 5

FILE *fAdd;
void *receive(void *arg);
pthread_t tids[N];

int main()
{
    // Declare socket file descriptor. All Unix I/O streams are referenced by descriptors
    int sockfd, connectfd, rb, sin_size;

    // Declare receiving and sending buffers of size 1k bytes
    char rbuf[1024], sbuf[1024];

    // Declare server address to which to bind for receiving messages and client address to fill in sending address
    struct sockaddr_in servAddr, clienAddr;

    // Open a TCP socket, if successful, returns a descriptor associated with an endpoint
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
        perror("Failure to setup an endpoint socket\n");
        exit(1);
    }

    // Setup the server address to bind using socket addressing structure
    servAddr.sin_family = AF_INET; // Internet protocol
    servAddr.sin_port = htons(5001); // Address port
    servAddr.sin_addr.s_addr = INADDR_ANY; // Internet address

    // Set address/port of server endpoint for socket socket descriptor
    if ((bind(sockfd, (struct sockaddr *)&servAddr, sizeof(struct sockaddr))) < 0)
	{
        perror("Failure to bind server address to the endpoint socket\n");
        exit(1);
    }
	
    printf("Server waiting for client\n");

	//if failure to connect
    if (listen(sockfd, N) == -1) 
	{
        perror("Failure to connect\n");
        exit(1);
    }

    for(int i = 0; i <= N; i++)
	{
        int *CONNECT = malloc(sizeof(int));
        *CONNECT = accept(sockfd, (struct sockaddr *)&clienAddr, (socklen_t *)&sin_size);
        pthread_create(&tids[i], NULL, receive, CONNECT);
    }
	
	//close file and socket
    fclose(fAdd);
    close(sockfd);
    return 0;
}

void *receive(void *arg)
{
    char recv_data[1024];
    int bytes_received;
    int connectfd = *(int*)arg;

    while((bytes_received = recv(connectfd, recv_data, 1024, 0)) > 0) 
	{
        //write received file into output.txt
        fAdd = fopen("output.txt", "a");
        if (!feof(fAdd))
		{
            fwrite(recv_data,1, bytes_received, fAdd);
        }
        else 
		{
            printf("Failure to open file\n");
        }
        fflush(fAdd);  
    }
    printf("Received Data\n");
	return 0;
}

