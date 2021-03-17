/*
 * Name: Evan Chou
 * Title: Lab3 Part 2
 * Description: This program coputes the socket from the UDP client side for the client to send a file to the server
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

int main(){
	//Declare socket file descriptor. All Unix I/O streams are referenced by descriptors
	 int sockfd; 

	 //Declare sending buffer of size 1k bytes
	 char sbuf[10000]; 
 
	 //Declare server address
	 struct sockaddr_in servAddr; 

	 //Converts domain names into numerical IP addresses via DNS
	 struct hostent *host;
	 host = (struct hostent *)gethostbyname("localhost"); //Local host runs the server. You may use "127.0.0.1": loopback IP address
 
	 //Open a socket, if successful, returns a descriptor associated with an endpoint 
	 if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	 {
		 perror("Failure to setup an endpoint socket");
		 exit(1);
	 }

	 //Set the server address to send using socket addressing structure
	 servAddr.sin_family = AF_INET;
	 servAddr.sin_port = htons(5000);
	 servAddr.sin_addr = *((struct in_addr *)host->h_addr);

	 FILE *fpr = fopen("number1to1000.txt", "r");
	
	 //find file size
	 fseek(fpr, 0L, SEEK_END); //end of file
	 size_t file_size=ftell(fpr); //tells us file size, so we know how much buffer to read
	 fseek(fpr,0,SEEK_SET); //beginning of file
	 fread(&sbuf, file_size, 1, fpr);

	 //Client to send file to the server continuously using UDP socket  
	 sendto(sockfd, sbuf, strlen(sbuf), 0, (struct sockaddr *) &servAddr, sizeof(struct sockaddr));

	 fclose(fpr);
	 return 0;
}
