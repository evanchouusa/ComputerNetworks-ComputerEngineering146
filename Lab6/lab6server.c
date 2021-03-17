/*
 * Name: Evan Chou
 * Date: October 30, 2020
 * Title: Lab6 - lab6server.c
 * Description: This program executes the server side, receiving a packet from client and checking checksum and sequence number, then sending an ACK or NAK back to client. 
 */

#include <arpa/inet.h>     
#include <stdio.h>         
#include <stdlib.h>         
#include <string.h>         
#include <sys/socket.h>     
#include <time.h>           
#include <unistd.h>         
#include <fcntl.h>          
#include "lab6.h"

int seq = 0;

//add the missing functions
//******STUDENT WORK******
int getChecksum(Packet packet) {
    packet.header.cksum = 0;
    int checksum = 0;
    char *ptr = (char *)&packet;
    char *end = ptr + sizeof(Header) + packet.header.len;
    while (ptr < end) {
        checksum ^= *ptr++;
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

void ServerSend(int sockfd, const struct sockaddr *address, socklen_t addrlen, int seqnum) {
    // Simulatting a chance that ACK gets lost
	if (rand()%10>2) { //80% chance get lose, 20% get through
        //******STUDENT WORK******
        printf("ACK lost\n");
        return;
    }
    //prepare and send the ACK
    Packet packet;
    //******STUDENT WORK******
    packet.header.seq_ack = seqnum;
    packet.header.len = sizeof(packet.data);
    packet.header.cksum = getChecksum(packet);

	//send packet
	sendto(sockfd, &packet, sizeof(packet), 0, address, addrlen);
    printf("Sent ACK %d, checksum %d\n", packet.header.seq_ack, packet.header.cksum);
}

Packet ServerReceive(int sockfd, struct sockaddr *address, socklen_t *addrlen, int seqnum) {
    Packet packet;

    while (1) {
		//Receive a packet from the client
        //******STUDENT WORK******
        recvfrom(sockfd, &packet, sizeof(packet), 0, address, addrlen);
		//logPacket(packet);

        // validate the length of the packet
        //******STUDENT WORK******
        int lengthOfPacket = strlen(packet.data) + 1;

        if (packet.header.len == 0) {
            lengthOfPacket = strlen(packet.data);
        }

        // log what was received
        printf("Received: ");
        logPacket(packet);

        //verify the checksum and the sequence number
        if (packet.header.cksum != getChecksum(packet)) {
            printf("Bad checksum, expected %d\n", getChecksum(packet));
            //******STUDENT WORK******
            ServerSend(sockfd, address, *addrlen, !seq);
        } else if (packet.header.seq_ack != seq) {
            printf("Bad seqnum, expected %d\n", seq);
            //******STUDENT WORK******
            ServerSend(sockfd, address, *addrlen, !seq);
        } else {
            printf("Good packet\n");
            //******STUDENT WORK******
            ServerSend(sockfd, address, *addrlen, seq);
            seq = !seq; 
        }

        printf("\n");
        return packet;
    }
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
    //******STUDENT WORK******
    int sockfd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(struct sockaddr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket Setup Failed\n");
        exit(1);
    }

    // initialize the server address structure
    //******STUDENT WORK******
    char *p;
    long portNum = 0;
    long conv = strtol(argv[1], &p, 10);
    portNum = conv;

    address.sin_family = AF_INET;
    address.sin_port = htons(portNum);
    address.sin_addr.s_addr = INADDR_ANY;

    // bind the socket
    //******STUDENT WORK******
    if ((bind(sockfd, (struct sockaddr *)&address, sizeof(struct sockaddr))) < 0){
        perror("Endpoint Socket Binding Failure\n");
        exit(1);
    }

    //Please answer the following questions
        //[Q1] Briefly explain what is "AF_INET" used for.
		//AF_INET is used to designate the type of address that my socket can communicate with. Basically it's the address family.
        //[Q2] Briefly explain what is "SOCK_DGRAM" used for.
		//SOCK_DGRAM is used for sending individually-addressed packets unreliably, since we're implementing UDP, not TCP, so unreliable data. So each time we write data to the socket, the data becomes one packet.
        //[Q3] Briefly explain what is "htons" used for.
		//htons is used to convert an IP port number in host byte order to the IP port number in network byte order. For instance, it takes a 16 bit number in host byte order and returns it in network byte order for out UDP network.
        //[Q4] Briefly explain why bind is required on the server and not on the client.
		//We don't need to bind on the client because the server is not expecting the client to come froma  specific port or port range. In terms of why we need to bind on the server, the server needs to bind to a known port such as 5003 so clients will know where to contact or connect to. This is why we run the server first, inputing a specific port number, and then run the client and type the same port number.
    
    // open file
	FILE *file = fopen(argv[2], "wb");
    
    // get file contents from client and save it to the file
    int seqnum = 0;
    Packet packet;
    struct sockaddr_in clientaddr;
    char pktData[10];
    socklen_t clientaddr_len = sizeof(clientaddr);

    do {
        //******STUDENT WORK******
        packet = ServerReceive(sockfd, (struct sockaddr*)&clientaddr, &clientaddr_len, packet.header.seq_ack);
		//printf("Packet header len: %d\n", packet.header.len);
		//printf("Packet data: %s", packet.data);
 		fwrite(packet.data, 1, packet.header.len, file);
    } while (packet.header.len != 0);

    //******STUDENT WORK******
    fclose(file);
    close(sockfd);
    
    return 0;
}
