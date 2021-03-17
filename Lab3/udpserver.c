/*
 * Name: Evan Chou
 * Title: Lab3 Part 2
 * Description: This program computes the socket from the UDP server side for the server to receive the file from the client and then prints the file in another destination
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(){
   	//Declare socket file descriptor. All Unix I/O streams are referenced by descriptors
	int sockfd;
 
	//Declare receiving buffer of size 1k bytes
	char rbuf[10000];

	//Declare server address to which to bind for receiving messages and client address to fill in sending address
	struct sockaddr_in servAddr, clienAddr;
	socklen_t addrLen = sizeof(struct sockaddr);

	//Open a UDP socket, if successful, returns a descriptor associated with an endpoint 
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		perror("Failure to setup an endpoint socket");
		exit(1);
	}
 
	//Setup the server address to bind using socket addressing structure
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(5000); //Port 5000 is assigned
	servAddr.sin_addr.s_addr = INADDR_ANY; //Local IP address of any interface is assigned (generally one interface IP address)

	//Set address/port of server endpoint for socket socket descriptor
	if((bind(sockfd, (struct sockaddr *)&servAddr, sizeof(struct sockaddr))) < 0)
	{
		perror("Failure to bind server address to the endpoint socket");
		exit(1);
	}
 
	FILE *destination = fopen("destination.txt", "w");

	//Sever continuously waits for files from client, then opens/writes file in destination.txt file
	while(1)
	{
		int nr = recvfrom(sockfd, rbuf, 10000, 0, (struct sockaddr *) &clienAddr, &addrLen);
		rbuf[nr] = '\0';
		fwrite(rbuf, 1, sizeof(rbuf), destination);
		fclose(destination);
	}
	return 0;
}
