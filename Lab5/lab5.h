/*
 * Name: Evan Chou
 * Date: October 23, 2020
 * Title: Lab5 - lab5.h
 * Description: This is a header file containing C declarations to be shared between the client and server files.
 */

#define PBADCKSUM 5 // P(bad checksum = 1) / PBADCKSUM
typedef struct{
    int seq_ack;
    int len;
    int cksum;
}Header;
typedef struct{
    Header header;
    char data[10];
}Packet;

int getChecksum(Packet packet);
void logPacket(Packet packet);
