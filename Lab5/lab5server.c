/*
 * Name: Evan Chou
 * Date: October 23, 2020
 * Title: Lab5 - server rdt 2.2
 * Description: This program waits for the client to send a packet and them compares the seqnum and checksum. Based on that, it sends an Ack or Nak response to client.
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "lab5.h"


// NOTE: beware of sign extension if the checksum is stored in a char, then converted to an int!
int getChecksum(Packet packet) {
    packet.header.cksum = 0;

    int checksum = 0;
    char *ptr = (char *)&packet;
    char *end = ptr + sizeof(Header) + packet.header.len;

//Please find an error from the remaining part getChecksum() function 
    while (ptr < end) {
        checksum ^= *ptr++; //change ptr to a pointer (*ptr) because we need to access the bit value themselves, not the actual address of it
    }

    return checksum;
}

void logPacket(Packet packet) {
    printf("Packet{ header: { seq_ack: %d, len: %d, cksum: %d }, data: \"",
            packet.header.seq_ack,
            packet.header.len,
            packet.header.cksum);
    fwrite(packet.data, (size_t)packet.header.len, 1, stdout);
    printf("\" }\n");
}

//Sending a ACK to the client, i.e., letting the client know what was the status of the packet it sent.
void ServerSend(int sockfd, const struct sockaddr *address, socklen_t addrlen, int seqnum) {
    Packet packet;
	//-Prepare the packet headers
	packet.header.seq_ack=seqnum;
	packet.header.len=sizeof(packet.data);
	packet.header.cksum=getChecksum(packet);
	//-send the packet
   	sendto(sockfd, &packet, sizeof(packet), 0, address, addrlen);
		

    printf("Sent ACK %d, checksum %d\n", packet.header.seq_ack, packet.header.cksum);
}

Packet ServerReceive(int sockfd, struct sockaddr *address, socklen_t *addrlen, int seqnum) {
    Packet packet;

    while (1) {
        //recv packets from the client
		recvfrom(sockfd, &packet, sizeof(packet), 0, address, addrlen);

        // log what was received
        printf("Received: ");
        logPacket(packet);

        if (packet.header.cksum != getChecksum(packet)) { //this is if checksum gives us an error
            printf("Bad checksum, expected checksum was: %d\n", getChecksum(packet));
            ServerSend(sockfd, address, *addrlen, !seqnum);
        } else if (packet.header.seq_ack != seqnum) {
            printf("Bad seqnum, expected sequence number was:%d\n", seqnum);
            ServerSend(sockfd, address, *addrlen, !seqnum);
        } else {
            printf("Good packet\n");
            ServerSend(sockfd, address, *addrlen, seqnum);
            break;
        }
    }

    printf("\n");
    return packet;
}


int main(int argc, char *argv[]) {
    // check arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <outfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // seed the RNG
    srand((unsigned)time(NULL));

    // create a socket
	int sockfd;
	if((sockfd=socket(AF_INET, SOCK_DGRAM, 0))<0){
		perror("Socket Setup Failed\n");
		exit(1);
	}

    // initialize the server address structure using argv[1]
    struct sockaddr_in address;
	address.sin_family=AF_INET;
	address.sin_port=htons(atoi(argv[1]));
	address.sin_addr.s_addr=INADDR_ANY;

    
	// [Question]: Do we need to bind the socket on the client side? Why?/Why not?
	// Answer: We do not need to bind the socket on the client side because there is no specific client port
	// bind the socket
	if((bind(sockfd,(struct sockaddr *)&address, sizeof(struct sockaddr)))<0){
		perror("Endpoint Socket Binding Failure\n");
		exit(1);
	}

    // open file using argv[2]
	int fp=open(argv[2], O_RDWR);

    // get file contents from client
    int seqnum = 0;
    Packet packet;
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    do {
        packet = ServerReceive(sockfd, (struct sockaddr *)&clientaddr, &clientaddr_len, seqnum);
		write(fp, packet.data, strlen(packet.data));
		seqnum=(seqnum+1)%2;
    } while (packet.header.len != 0);

    close(fp);
    close(sockfd);
    return 0;
}
