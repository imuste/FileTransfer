/*****************************************************************************
 *
 *      client.c
 *
 *      Isabel Muste (imuste01)
 *      10/30/2024
 *      
 *      CS 112 HW03
 * 
 *      ...
 *      
 *
 *****************************************************************************/

#include <math.h> 
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

typedef struct __attribute__((__packed__)) 
{
        char msgType;
        char winSeq;
        char data[512];
        
} message;


typedef struct 
{
        char *serverIP;
        int serverPort;
        struct sockaddr* serverAddr;

        int clientSD;

} info;


void setupServerConn(info *theInfo);
void writeRRQ(info *theInfo);
void readData(info *theInfo);

void printHeader(message header);


/*
 * name:      main
 * purpose:   opens the commands file and initializes a new cache instance
 * arguments: argc, argv
 * returns:   exit success
 * effects:   none
 */
int main(int argc, char *argv[])
{       
        info *theInfo = malloc(sizeof(info));
        theInfo->serverIP = malloc(20);
        theInfo->serverAddr = (struct sockaddr *)malloc(sizeof(struct sockaddr));

        /* check command line arguments */
        if (argc == 1) {
                memcpy(theInfo->serverIP, "10.4.2.18\0", 11);
                theInfo->serverPort = 9099;
        }
        else if (argc == 2) {
                memcpy(theInfo->serverIP, "10.4.2.18\0", 11);
                theInfo->serverPort = atoi(argv[1]);
        }
        else {
                memcpy(theInfo->serverIP, argv[2], strlen(argv[2]));
                theInfo->serverPort = atoi(argv[1]);
        }

        setupServerConn(theInfo);
        writeRRQ(theInfo);
        readData(theInfo);


        //close the socket
        shutdown(theInfo->clientSD, SHUT_RDWR);
        close(theInfo->clientSD);
        
        return EXIT_SUCCESS;
}


void setupServerConn(info *theInfo)
{
        int clientFD;
        int serverAddrLen;
        struct sockaddr_in serverAddr;
        struct hostent *server;
        char *hostName;

        // socket: create the socket
        clientFD = socket(AF_INET, SOCK_DGRAM, 0);
        if (clientFD < 0) {
                fprintf(stderr,"ERROR opening socket");
        }
                
        server = gethostbyname(theInfo->serverIP);
        assert(server != NULL);

        // build the server's Internet address
        bzero((char *)&serverAddr, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        memcpy(&serverAddr.sin_addr.s_addr, server->h_addr, server->h_length);
        serverAddr.sin_port = htons(theInfo->serverPort);

        theInfo->serverAddr = (struct sockaddr *)&serverAddr;
        theInfo->clientSD = clientFD;

}


void writeRRQ(info *theInfo)
{
        message header1;
        header1.msgType = 1;
        header1.winSeq = 1;
        strcpy(header1.data, "filex.txt");
        header1.data[sizeof(header1.data) - 1] = '\0';

        int serverAddrLen = sizeof(struct sockaddr_in);
        int returnVal = sendto(theInfo->clientSD, (void *)&header1, 
                sizeof(header1), 0, (struct sockaddr *)theInfo->serverAddr, serverAddrLen);
        assert(returnVal != -1);
}


void readData(info *theInfo)
{
        // print the server's reply 
        message receivedMessage;
        int bufferSize = sizeof(message);
        int serverAddrLen = sizeof(struct sockaddr_in);
        int returnVal = recvfrom(theInfo->clientSD, &receivedMessage, bufferSize, 0, 
                theInfo->serverAddr, &serverAddrLen);
        assert(returnVal != -1);

        printHeader(receivedMessage);

}


void printHeader(message header) 
{
        printf("\n");
        if (header.msgType == 1) {
                printf("RRQ\n");
        }
        else if (header.msgType == 2) {
                printf("DATA\n");
        }
        else if (header.msgType == 3) {
                printf("ACK\n");
        }
        else if (header.msgType == 4) {
                printf("ERROR\n");
        }
        printf("Message Header:\n");
        printf("  msgType: %d\n", header.msgType);
        printf("\n");
}
