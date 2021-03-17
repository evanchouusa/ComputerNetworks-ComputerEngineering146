/*
 * Name: Evan Chou
 * Date: October 23, 2020
 * Title: Lab5 - client rdt 2.2
 * Description: This program sends out a packet with seqnum and checksum to server. It then waits for the server's response and sends out the next packet depending whether it is good ack or nak.
 */

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include "lab5.h"

int testsum=0;
int seqnum=0;

// NOTE: beware of sign extension if the checksum is stored in a char, then converted to an int!
int getChecksum(Packet packet) {
    packet.header.cksum = 0;

    int checksum = 0;
    char *ptr = (char *)&packet;
    char *end = ptr + sizeof(Header) + packet.header.len;
 
    while (ptr < end) {
        checksum ^= *ptr++; //change ptr to a pointer (*ptr) because we need to access the bit values themselves, not the actual address of it
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



void ClientSend(int sockfd, const struct sockaddr *address, socklen_t addrlen, Packet packet) {
	while (1) {
		
		int n=rand()%10;
   		packet.header.seq_ack=seqnum;
        packet.header.len=strlen(packet.data);
 
        if(n>5){
            packet.header.cksum=getChecksum(packet);
        }

        // send the packet
        printf("Sending packet\n");
		sendto(sockfd, &packet, sizeof(packet), 0, address, addrlen);

        // receive an ACK from the server
        Packet recvpacket;
		recvfrom(sockfd, &recvpacket, sizeof(recvpacket), 0, address, &addrlen);

        // log what was received
        printf("Received ACK %d, checksum %d - ", recvpacket.header.seq_ack, recvpacket.header.cksum);
 
		testsum++;
        // validate the ACK
        if (recvpacket.header.cksum != getChecksum(recvpacket) || testsum%3==2) { //this is if checksum gives us an error of the receiver packet header checksum not being identical
            // bad checksum, resend packet
            printf("Bad checksum, expected checksum was: %d\n", getChecksum(recvpacket));
        } else if (recvpacket.header.seq_ack != packet.header.seq_ack) {
            // incorrect sequence number, resend packet
            printf("Bad seqnum, expected sequence number was: %d\n", packet.header.seq_ack);
        } else {
            // good ACK, we're done
            printf("Good ACK\n");
            break;
        }
    }

    printf("\n");
}

int main(int argc, char *argv[]) {
    // check arguments
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <ip> <port> <infile>\n", argv[0]);
        return 1;
    }

    // seed the RNG
    srand((unsigned)time(NULL));

    // create and configure the socket [name the socket sockfd] 
	int sockfd;
	if((sockfd=socket(AF_INET, SOCK_DGRAM, 0))<0){
		perror("Socket Setup Failed\n");
		exit(1);
	}


    // initialize the server address structure using argv[2] and argv[1]
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
	
	socklen_t addr_len=sizeof(address);
	struct hostent *host;
	host=(struct hostent *)gethostbyname(argv[1]);
	
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(argv[2]));
	address.sin_addr = *((struct in_addr *)host->h_addr);

    // open file using argv[3]
	int fp=open(argv[3], O_RDWR);

    // send file contents to server
	Packet packet; //-create a packet
	
	//read file to send to server
	while(read(fp, packet.data, sizeof(packet.data))>0){ 
		ClientSend(sockfd, (struct sockaddr *)&address, addr_len, packet); //-send the packet
		seqnum=(seqnum+1)%2; //update the sequence number
	}

    // send zero-length packet to server to end connection
	Packet zerolength;

	zerolength.header.seq_ack=seqnum;
	zerolength.header.len=0;
	zerolength.header.cksum=getChecksum(zerolength);
	ClientSend(sockfd, (struct sockaddr *)&address, addr_len, zerolength);

    // clean up
	//-close the sockets and file
	close(fp);
	close(sockfd);

    return 0;
}

