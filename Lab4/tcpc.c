/*
 * Name: Evan Chou
 * Title: Lab4 TCP client
 * Date: October 16, 2020
 * Description: This program runs the TCP client for TCP client-server distribution
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int main()
{
    // Declare socket file descriptor. All Unix I/O streams are referenced by descriptors. nr is a declaration of the number of recieved bytes.
    int sockfd, nr;

    // Declare sending and recieving buffers of size 1k bytes
    char sbuf[1024], rbuf[1024];

    // Declare server address to connect to
    struct sockaddr_in servAddr;
    struct hostent *host;

    // Converts domain names into numerical IP addresses via DNS
    host = (struct hostent *)gethostbyname("localhost");

    //Open a socket, if successful, returns a descriptor associated with an endpoint
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
        perror("Failure to setup an endpoint socket\n");
        exit(1);
    }

    // Set the server address to send using socket addressing structure
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(5001);
    servAddr.sin_addr = *((struct in_addr *)host->h_addr);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&servAddr, sizeof(struct sockaddr)))
	{
        perror("Failure to connect to the server\n");
        exit(1);
    }

	//Client begins to read text file
    FILE *fpsend;
    fpsend = fopen("number.txt", "r"); //open and read number.txt
    if (fpsend == NULL)
	{
        printf("Cannot open file\n");
        exit(0);
    }

    while (!feof(fpsend)) 
	{
        int button = fread(sbuf, 1, sizeof(sbuf), fpsend);
        //Client sends to server the file
        sendto(sockfd, sbuf, button, 0, (struct sockaddr *)&servAddr, sizeof(struct sockaddr));
    }

    // Close socket descriptor
    close(sockfd);
    return 0;
}
