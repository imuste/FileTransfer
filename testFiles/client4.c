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
void readData1(info *theInfo);
void writeAck0(info *theInfo);
void readData2(info *theInfo);
void readData3(info *theInfo);
void readData4(info *theInfo);
void writeAck2(info *theInfo);
void readData5(info *theInfo);
void readData6(info *theInfo);
void writeAck5(info *theInfo);
void readData7(info *theInfo);
void readData8(info *theInfo);
void writeAck6(info *theInfo);

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
        readData1(theInfo);
        writeAck0(theInfo);
        readData2(theInfo);
        // printf("SLEEPING 6 SEC\n");
        sleep(6);
        readData3(theInfo);
        readData4(theInfo);
        writeAck2(theInfo);
        readData5(theInfo);
        // printf("SLEEPING 3 SEC\n");
        sleep(3);
        readData6(theInfo);
        writeAck5(theInfo);
        readData7(theInfo);
        // printf("SLEEPING 3 SEC\n");
        sleep(3);
        readData8(theInfo);
        writeAck6(theInfo);


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

        // create the socket
        clientFD = socket(AF_INET, SOCK_DGRAM, 0);
        assert(clientFD != -1);
                
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
        header1.winSeq = 3;
        strcpy(header1.data, "client4_GT.txt");
        header1.data[sizeof(header1.data) - 1] = '\0';

        int serverAddrLen = sizeof(theInfo->serverAddr);
        int returnVal = sendto(theInfo->clientSD, (void *)&header1, 
                sizeof(header1), 0, (struct sockaddr *)&theInfo->serverAddr, 
                serverAddrLen);
        assert(returnVal != -1);

        theInfo->fileContents = (char *)malloc(2793);
        assert(theInfo->fileContents != NULL);
        theInfo->bytesReceived = 0;
}


// Read packets 0, 1, 2 + store in buffer
void readData1(info *theInfo)
{
        int serverAddrLen = sizeof(theInfo->serverAddr);
        message *receivedMessage = malloc(sizeof(message));
        assert(receivedMessage != NULL);

        int returnVal = -1;
        int packetsReceived = 0;
        while (packetsReceived < 3) {
        
                returnVal = recvfrom(theInfo->clientSD, receivedMessage, 
                        sizeof(message), 0, (struct sockaddr *)&theInfo->serverAddr, 
                        &serverAddrLen);
                assert(returnVal != -1 && returnVal != 0);
                assert(receivedMessage != NULL);

                // printHeader(receivedMessage);

                memcpy(theInfo->fileContents + theInfo->bytesReceived, 
                        ((message *)receivedMessage)->data, returnVal - 2);
                theInfo->bytesReceived += (returnVal - 2);

                packetsReceived += 1;
        }
}

// sent ACK 0
void writeAck0(info *theInfo)
{
        message header1;
        header1.msgType = 3;
        header1.winSeq = 0;

        int serverAddrLen = sizeof(theInfo->serverAddr);
        int returnVal = sendto(theInfo->clientSD, (void *)&header1, 
                sizeof(header1), 0, (struct sockaddr *)&theInfo->serverAddr, 
                serverAddrLen);
        assert(returnVal != -1);
}


// Read packet 3 + store in buffer
void readData2(info *theInfo)
{
        int serverAddrLen = sizeof(theInfo->serverAddr);

        message *receivedMessage = malloc(sizeof(message));
        assert(receivedMessage != NULL);

        int returnVal = -1;
        returnVal = recvfrom(theInfo->clientSD, receivedMessage, 
                sizeof(message), 0, (struct sockaddr *)&theInfo->serverAddr, 
                &serverAddrLen);
        assert(returnVal != -1 && returnVal != 0);
        assert(receivedMessage != NULL);

        // printHeader(receivedMessage);

        memcpy(theInfo->fileContents + theInfo->bytesReceived, 
                ((message *)receivedMessage)->data, returnVal - 2);
        theInfo->bytesReceived += (returnVal - 2);
}


// Read packets 1, 2, 3
void readData3(info *theInfo)
{
        int serverAddrLen = sizeof(theInfo->serverAddr);
        message *receivedMessage = malloc(sizeof(message));
        assert(receivedMessage != NULL);

        int returnVal = -1;
        int packetsReceived = 0;
        while (packetsReceived < 3) {
        
                returnVal = recvfrom(theInfo->clientSD, receivedMessage, 
                        sizeof(message), 0, (struct sockaddr *)&theInfo->serverAddr, 
                        &serverAddrLen);
                assert(returnVal != -1 && returnVal != 0);
                assert(receivedMessage != NULL);

                // printHeader(receivedMessage);
                packetsReceived += 1;
        }
}


// Read packets 1, 2, 3 again
void readData4(info *theInfo)
{
        int serverAddrLen = sizeof(theInfo->serverAddr);
        message *receivedMessage = malloc(sizeof(message));
        assert(receivedMessage != NULL);

        int returnVal = -1;
        int packetsReceived = 0;
        while (packetsReceived < 3) {
        
                returnVal = recvfrom(theInfo->clientSD, receivedMessage, 
                        sizeof(message), 0, (struct sockaddr *)&theInfo->serverAddr, 
                        &serverAddrLen);
                assert(returnVal != -1 && returnVal != 0);
                assert(receivedMessage != NULL);

                // printHeader(receivedMessage);
                packetsReceived += 1;
        }
}

// write ACK 2
void writeAck2(info *theInfo)
{
        message header1;
        header1.msgType = 3;
        header1.winSeq = 2;

        int serverAddrLen = sizeof(theInfo->serverAddr);
        int returnVal = sendto(theInfo->clientSD, (void *)&header1, 
                sizeof(header1), 0, (struct sockaddr *)&theInfo->serverAddr, 
                serverAddrLen);
        assert(returnVal != -1);
}


// Read packets 4, 5 + store in buffer
void readData5(info *theInfo)
{
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

                packetsReceived += 1;
        }
}


// Read packets 3, 4, 5 again
void readData6(info *theInfo)
{
        int serverAddrLen = sizeof(theInfo->serverAddr);
        message *receivedMessage = malloc(sizeof(message));
        assert(receivedMessage != NULL);

        int returnVal = -1;
        int packetsReceived = 0;
        while (packetsReceived < 3) {
        
                returnVal = recvfrom(theInfo->clientSD, receivedMessage, 
                        sizeof(message), 0, (struct sockaddr *)&theInfo->serverAddr, 
                        &serverAddrLen);
                assert(returnVal != -1 && returnVal != 0);
                assert(receivedMessage != NULL);

                // printHeader(receivedMessage);
                packetsReceived += 1;
        }
}

// write ACK 5
void writeAck5(info *theInfo)
{
        message header1;
        header1.msgType = 3;
        header1.winSeq = 5;

        int serverAddrLen = sizeof(theInfo->serverAddr);
        int returnVal = sendto(theInfo->clientSD, (void *)&header1, 
                sizeof(header1), 0, (struct sockaddr *)&theInfo->serverAddr, 
                serverAddrLen);
        assert(returnVal != -1);
}


// Read packet 6 + store in buffer
void readData7(info *theInfo)
{
        int serverAddrLen = sizeof(theInfo->serverAddr);

        message *receivedMessage = malloc(sizeof(message));
        assert(receivedMessage != NULL);

        int returnVal = -1;
        returnVal = recvfrom(theInfo->clientSD, receivedMessage, 
                sizeof(message), 0, (struct sockaddr *)&theInfo->serverAddr, 
                &serverAddrLen);
        assert(returnVal != -1 && returnVal != 0);
        assert(receivedMessage != NULL);

        // printHeader(receivedMessage);

        memcpy(theInfo->fileContents + theInfo->bytesReceived, 
                ((message *)receivedMessage)->data, returnVal - 2);
        theInfo->bytesReceived += (returnVal - 2);
}


// Read packet 6 again
void readData8(info *theInfo)
{
        int serverAddrLen = sizeof(theInfo->serverAddr);

        message *receivedMessage = malloc(sizeof(message));
        assert(receivedMessage != NULL);

        int returnVal = -1;
        returnVal = recvfrom(theInfo->clientSD, receivedMessage, 
                sizeof(message), 0, (struct sockaddr *)&theInfo->serverAddr, 
                &serverAddrLen);
        assert(returnVal != -1 && returnVal != 0);
        assert(receivedMessage != NULL);

        // printHeader(receivedMessage);
}


// write ACK 6
void writeAck6(info *theInfo)
{
        message header1;
        header1.msgType = 3;
        header1.winSeq = 6;

        int serverAddrLen = sizeof(theInfo->serverAddr);
        int returnVal = sendto(theInfo->clientSD, (void *)&header1, 
                sizeof(header1), 0, (struct sockaddr *)&theInfo->serverAddr, 
                serverAddrLen);
        assert(returnVal != -1);


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
}
