/*****************************************************************************
 *
 *      server.h
 *
 *      Isabel Muste (imuste01)
 *      10/30/2024
 *      
 *      CS 112 HW03
 * 
 *      the server interface can be used to connect clients to a server and
 *      allow them to initiate file transfers
 *
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <math.h> 
#include <limits.h>
#include <time.h>

#include <sys/un.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifndef SERVER_H


typedef struct __attribute__((__packed__)) 
{
        char msgType;
        char winSeq;
        char data[512];
        
} message;



typedef struct 
{
        int portNumber;
        int socketFD;
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength;

        message *clientMsg;

        char *fileContents;
        int fileSize;
        int numPackets;
        int packetsSent;
        int mostRecentPkt;
        int *sentPkt;

} server;



/*****************************************************************
*                    FUNCTION DECLARATIONS
*****************************************************************/
server *newServer(int port);
void runFileTransfers(server *theServer);
void processClientRequest(server *theServer);

// Receiving Functions
void receiveRRQ(server *theServer);
bool receiveAck(server *theServer);

// Sending Functions
void sendWindow(server *theServer);
void sendPacket(server *theServer, int packNum, message *dataMessage, 
        int dataSize);
void sendErrorMessage(server *theServer);

// File Functions
bool getRequestedFile(server *theServer);
void setFileInformation(server *theServer);

// Helper Functions
void createSocket(server *theServer);
bool clientTimeout(server *theServer);
void freeClientInfo(server *theServer);
bool checkRRQFields(server *theServer);
void checkNegOneError(int returnVal);
void checkAllocError(char *memAllocated);


// void printMessage(message *header);

#endif