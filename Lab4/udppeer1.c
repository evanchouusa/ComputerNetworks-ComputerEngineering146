/*
 * Name: Evan Chou
 * Title: Lab4 UDP Peer1
 * Date: October 16, 2020
 * Description: This program runs the udp peer1 for peer2peer file distribution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#define SIZE 1000

//Declare socket file descriptor. All
int sockfd;

//Declare sending and receiving buffer of size 1k bytes
char sbuf[1024], rbuf[1024];

//Declare server address to which to bind for receiving messages and client address to fill in sending address
struct sockaddr_in servAddr1, servAddr2;
socklen_t addrLen = sizeof(struct sockaddr);

struct hostent *host;
char host_name[20], fsrc[20], fdest[20];

void openSocket() {
    printf("Opening up the socket\n");
    // host = (struct hostent *)gethostbyname(host_name);
    host = (struct hostent *)gethostbyname("localhost");
    if (host == NULL) {
        printf("Failure to resolve hostname");
        exit(1);
    }

    //Set the server address to send using socket addressing structure and send port 5001
    servAddr1.sin_family = AF_INET;
    servAddr1.sin_port = htons(5001);
    servAddr1.sin_addr = *((struct in_addr *)host->h_addr); 
    

    //Setup the server address to bind using socket addressing structure, this is server2, port 5002
    servAddr2.sin_family = AF_INET;
    servAddr2.sin_port = htons(5002); 
    servAddr2.sin_addr = *((struct in_addr *)host->h_addr); 

    //Test if socket failed or is successful
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Failure to setup an endpoint socket");
        exit(1);
    }
}

//This is the function for the server side of the peer who wants to send the requested document to the client peer
void server() {
    printf("Server Function:\n");
    gethostname(host_name, 20);
    openSocket();

    //If server address does not bind to endpoint socket, then return failure
    if ((bind(sockfd, (struct sockaddr *)&servAddr1, sizeof(struct sockaddr))) < 0){
        printf("Bind socket failure\n");
        exit(1);
    }

    int rb;
    while((rb = recvfrom(sockfd, rbuf, 1024, 0, (struct sockaddr *)&servAddr2, &addrLen)) > 0) 
	{
        rbuf[rb] = '\0';
        printf("client %s\n", rbuf);
        if(strcmp(rbuf, "request") == 0) 
		{
            break;
        }
    }

    //opening file source
    int fp = open(fsrc, O_RDWR);
	//This is if we can't open file source, then tell user failure to open file source
    if (fp < 0) {
        printf("Failure to open file source\n");
        sendto(sockfd, "FAILURE", strlen("FAILURE"), 0, (struct sockaddr *)&servAddr2, sizeof(struct sockaddr));
        exit(1);
    }

    sendto(sockfd, "SENDING", strlen("SENDING"), 0, (struct sockaddr *)&servAddr2, sizeof(struct sockaddr));

    //Server sends file to client
    int bytes_read;
    while (0 < (bytes_read = read(fp, sbuf, sizeof(sbuf)))) {
        sendto(sockfd, sbuf, bytes_read, 0, (struct sockaddr *)&servAddr2, sizeof(struct sockaddr));
    }

    sendto(sockfd, "EndOfFile", sizeof("EndOfFile"), 0, (struct sockaddr *)&servAddr2, sizeof(struct sockaddr));

    //close file and socket
    close(fp);
    printf("File Finished\n");
    close(sockfd);
}

//This is the function for the client peer to request a certain file from its fellow server peer
void client() {
    printf("Client Function:\n");
    printf("Enter server hostname to receive from: ");
    scanf("%s", host_name);
    openSocket();

    //client requests for certain file
    sendto(sockfd, "request", strlen("request"), 0, (struct sockaddr *)&servAddr2, sizeof(struct sockaddr));
    printf("File request sent\n");

    //wait for response
    int rb;
    while ((rb = recvfrom(sockfd, rbuf, 1024, 0, (struct sockaddr *)&servAddr2, &addrLen)) > 0) {
        rbuf[rb] = '\0';
        printf("Message sent to server: %s\n", rbuf);
        if (strcmp(rbuf, "SENDING") == 0) {
            break;
        } else if (strcmp(rbuf, "FAILURE") == 0) {
            printf("Cannot open src in servAddr2\n");
            exit(1);
        }
    }

    int fp = open(fdest, O_RDWR | O_CREAT, 0777);
    if (fp < 0) {
        printf("Cannot open src \n");
        exit(1);
    }

    //receive file and break if received
    while ((rb = recvfrom(sockfd, rbuf, 1024, 0, (struct sockaddr *)&servAddr2, &addrLen)) > 0) {
        rbuf[rb] = '\0';
        if (strcmp(rbuf, "EndOfFile") == 0) {
            printf ("End Of File\n");
            break;
        } else {
            write(fp, rbuf, strlen(rbuf));
        }
    }

    //close file and socket
    close(fp);
    printf("File Finished!\n");
    close(sockfd);
}

int main() {
    while (1) {
        char input;
		printf("This is peer 1, Type S to send or R to receive, and Q to quit: \n");
        scanf("%c", &input);
        if (input == 'S') {
            printf("Peer 1 sending!\n");
            printf("Please enter the filename you want to send: ");
            scanf("%s", fsrc);
            server();
        } else if (input == 'R') {
            printf("Peer 1 receiving\n");
            printf("Please enter the filename you want to receive to: ");
            scanf("%s", fdest);
            client();
        } else if (input == 'Q') {
            exit(1);
        }
    }
    return 0;
}
