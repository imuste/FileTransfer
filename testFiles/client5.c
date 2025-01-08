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
        struct sockaddr_in serverAddr;

        int clientSD;
        char *fileContents;
        int bytesReceived;

} info;


void setupServerConn(info *theInfo);
void writeRRQ(info *theInfo);
void readData(info *theInfo);
void readData2(info *theInfo);

void printHeader(message *header);


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
        // theInfo->serverAddr = malloc(sizeof(struct sockaddr_in));

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

        sleep(15);

        for (int i = 0; i < 4; i++) {
                readData(theInfo);
        }

        readData2(theInfo);


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

        theInfo->serverAddr = serverAddr;
        theInfo->clientSD = clientFD;

}


void writeRRQ(info *theInfo)
{
        message header1;
        header1.msgType = 1;
        header1.winSeq = 4;
        strcpy(header1.data, "client5_GT.txt");
        header1.data[sizeof(header1.data) - 1] = '\0';

        int serverAddrLen = sizeof(theInfo->serverAddr);
        int returnVal = sendto(theInfo->clientSD, (void *)&header1, 
                sizeof(header1), 0, (struct sockaddr *)&theInfo->serverAddr, 
                serverAddrLen);
        assert(returnVal != -1);
}


void readData(info *theInfo)
{
        // print the server's reply 
        int serverAddrLen = sizeof(theInfo->serverAddr);

        message *receivedMessage = malloc(sizeof(message));
        assert(receivedMessage != NULL);

        int returnVal = -1;
        int packetsReceived = 0;
        while (packetsReceived < 2) {
        
                returnVal = recvfrom(theInfo->clientSD, receivedMessage, 
                        sizeof(message), 0, (struct sockaddr *)&theInfo->serverAddr, 
                        &serverAddrLen);
                assert(returnVal != -1 && returnVal != 0);
                assert(receivedMessage != NULL);

                // printHeader(receivedMessage);

                packetsReceived += 1;
        }
}


void readData2(info *theInfo)
{
        theInfo->fileContents = (char *)malloc(882);
        assert(theInfo->fileContents != NULL);
        theInfo->bytesReceived = 0;

        // print the server's reply 
        int serverAddrLen = sizeof(theInfo->serverAddr);

        message *receivedMessage = malloc(sizeof(message));
        assert(receivedMessage != NULL);

        int returnVal = -1;
        int packetsReceived = 0;
        while (packetsReceived < 2) {
        
                returnVal = recvfrom(theInfo->clientSD, receivedMessage, 
                        sizeof(message), 0, (struct sockaddr *)&theInfo->serverAddr, 
                        &serverAddrLen);
                assert(returnVal != -1 && returnVal != 0);
                assert(receivedMessage != NULL);

                // printHeader(receivedMessage);

                memcpy(theInfo->fileContents + theInfo->bytesReceived, 
                        ((message *)receivedMessage)->data, returnVal - 2);
                theInfo->bytesReceived += (returnVal - 2);

                packetsReceived++;
        }
        // printf("bytesReceived: %d\n", theInfo->bytesReceived);
        printf("%s", theInfo->fileContents);
}




void printHeader(message *header) 
{
        printf("\n");
        if (header->msgType == 1) {
                printf("RRQ\n");
        }
        else if (header->msgType == 2) {
                printf("DATA\n");
        }
        else if (header->msgType == 3) {
                printf("ACK\n");
        }
        else if (header->msgType == 4) {
                printf("ERROR\n");
        }
        printf("Message Header:\n");
        printf("  msgType: %d\n", header->msgType);
        printf("  winSize: %d\n", header->winSeq);
        printf("  data: %s\n", header->data);
        printf("\n");

        // printf("%s\n", header->data);

        // sleep(1);
}
