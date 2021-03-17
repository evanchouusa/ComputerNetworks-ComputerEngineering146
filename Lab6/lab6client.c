/*
 * Name: Evan Chou
 * Date: October 30, 2020
 * Title: Lab6 - lab6client.c
 * Description: This program executes the client side, sending out a packet with checksum and sequence number and then waiting for server's ACK. The stop-and-wait protocol and time out is also implemented. 
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
#include <netdb.h>
#include "lab6.h"

int seq = 0;

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

void ClientSend(int sockfd, const struct sockaddr *address, socklen_t addrlen, Packet packet, unsigned retries) {
    int attempts = 0;
    while (attempts < retries) {
        //log what is being sent
        printf("Created: ");
		//logPacket(packet);

        // Simulate lost packet.
        if (rand()%(10) + 1 == 0) {
            printf("Dropping packet\n");
        } else {
            //******STUDENT WORK******
            sendto(sockfd, &packet, sizeof(packet), 0, address, addrlen);
            printf("Sent this packet: \n");
            logPacket(packet);
            }

        //wait until either a packet is received or timeout
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        fd_set readfds;
        //******STUDENT WORK******
        fcntl(sockfd, F_SETFL, O_NONBLOCK);

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        int rv;
        rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);

        if (rv == 0) { //Timeout occured, so resend packet
            printf("TIMEOUT, RESEND PACKET \n");
            attempts++;
        } else if (rv == 1) {
            //receive an ACK from the server
            Packet recvpacket;
            //******STUDENT WORK******
            recvfrom(sockfd, &recvpacket, sizeof(packet), 0, NULL, NULL); //receive packet from server

            //print out what was received
            printf("Received ACK %d, checksum %d - ", recvpacket.header.seq_ack, recvpacket.header.cksum);

            //check ACK
            if (recvpacket.header.cksum != getChecksum(recvpacket)) {
                //means bad checksum, so resend packet
                printf("Bad checksum, expected %d\n", getChecksum(recvpacket));
                ClientSend(sockfd, address, addrlen, packet, retries);
            } else if (recvpacket.header.seq_ack != seq) {
                //means wrong sequence number, so resend packet
                printf("Bad seqnum, expected %d\n", packet.header.seq_ack);
                ClientSend(sockfd, address, addrlen, packet, retries);
            } else {
                //Good ACK
                printf("Good ACK\n");
                seq = !seq;
                break;
            }
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

    // create and configure the socket
    //******STUDENT WORK******
    int sockfd;
    char sbuf[10];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failure to setup an endpoint socket");
        exit(1);
    }

    // initialize the server address structure
    //******STUDENT WORK******
    struct sockaddr_in servAddr;

    struct hostent *host;
    host = (struct hostent *)gethostbyname(argv[1]);

    char *p;
    int portNum = 0;
    long conv = strtol(argv[2], &p, 10);
    portNum = conv;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(portNum);
    servAddr.sin_addr = *((struct in_addr *)host->h_addr);

    // open file
    int file = open(argv[3], O_RDWR);

    // send file contents to server
    Packet packet;
    //******STUDENT WORK******
    int bytes_read;
    while ((bytes_read = read(file, packet.data, sizeof(packet.data))) != 0) {
        packet.header.seq_ack = seq;
        packet.header.len = bytes_read;
        packet.header.cksum = getChecksum(packet); //calculate checksum 
        ClientSend(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr), packet, 3); //sending in max 3 timeout resends
    }

    // send zero-length packet to server to end connection according the description provided in the Lab manual
    //******STUDENT WORK******
    packet.header.len = 0;
    packet.header.seq_ack = seq;
    packet.header.cksum = getChecksum(packet); 
    ClientSend(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr), packet, 3); //sending in max 3 timeout resends

    //******STUDENT WORK******
    close(file);
    close(sockfd);

    return 0;
}
