/*****************************************************************************
 *
 *      server.c
 *
 *      Isabel Muste (imuste01)
 *      10/30/2024
 *      
 *      CS 112 HW03
 * 
 *      contains the function definitions for the server interface
 *      
 *
 *****************************************************************************/

#include "server.h"


/*****************************************************************************
*                               SERVER SETUP
******************************************************************************/

/*
 * name:      newServer
 * purpose:   allocates memory for a new server instance
 * arguments: the server port number specified by the user
 * returns:   the server instance
 * effects:   none
 */
server *newServer(int port)
{
        server *theServer = malloc(sizeof(server));
        checkAllocError((char *)theServer);
        theServer->portNumber = port;
        theServer->socketFD = -1;

        theServer->clientMsg = NULL;
        theServer->fileContents = NULL;
        theServer->fileSize = 0;
        theServer->numPackets = 0;
        theServer->packetsSent = 0;
        theServer->mostRecentPkt = -1;
        theServer->sentPkt = NULL;
        
        return theServer;
}


/*
 * name:      runFileTransfers
 * purpose:   creates the socket and processes incoming clients
 * arguments: the server instance
 * returns:   none
 * effects:   frees client information before handling new client
 */
void runFileTransfers(server *theServer)
{       
        createSocket(theServer);
        theServer->clientAddressLength = sizeof(theServer->clientAddress);
        while (true) {
                theServer->clientMsg = malloc(sizeof(message));
                processClientRequest(theServer);
                freeClientInfo(theServer);
        }
}


/*
 * name:      processClientRequest
 * purpose:   processes the client request by sending the file
 * arguments: the server instance
 * returns:   none
 * effects:   none
 */
void processClientRequest(server *theServer)
{
        // get request and open file
        receiveRRQ(theServer);
        if (!checkRRQFields(theServer)) {
                return;
        }
        if (!getRequestedFile(theServer)) {
                sendErrorMessage(theServer);
                return;
        }
        setFileInformation(theServer);

        // send all packets or timeout and stop connection
        while (theServer->packetsSent < theServer->numPackets) {
                sendWindow(theServer);

                bool ackReceived = false;
                for (int i = 0; i < 5; i++) {
                        if (receiveAck(theServer)) {
                                ackReceived = true;
                                break;
                        }
                        if (i != 4) {
                                sendWindow(theServer);
                        }
                }
                if (!ackReceived) {
                        return;
                }
        }
}


/*****************************************************************************
*                           RECEIVING FUNCTIONS
******************************************************************************/


/*
 * name:      receiveRRQ
 * purpose:   does a recvfrom call to get the RRQ from the client
 * arguments: the server instance
 * returns:   none
 * effects:   allocates memory for client information
 */
void receiveRRQ(server *theServer)
{
        struct hostent *hostInfo;
        char *hostAddrStr;

        // receive a UDP datagram from a client
        memset(theServer->clientMsg, 0, sizeof(message));
        int returnVal = recvfrom(theServer->socketFD, theServer->clientMsg, 
                sizeof(message), 0, (struct sockaddr *)&theServer->clientAddress, 
                &theServer->clientAddressLength);
        checkNegOneError(returnVal);

        // determine who sent the datagram
        hostInfo = gethostbyaddr(
                (const char *)&theServer->clientAddress.sin_addr.s_addr, 
                sizeof(theServer->clientAddress.sin_addr.s_addr), 
                AF_INET);
        checkAllocError((char *)hostInfo);

        hostAddrStr = inet_ntoa(theServer->clientAddress.sin_addr);
        checkAllocError((char *)hostAddrStr);
}


/*
 * name:      receiveAck
 * purpose:   uses the select call to wait for an ACK from the client and 
 *            times out if no ACK comes within 3 seconds
 * arguments: the server instance
 * returns:   none
 * effects:   none
 */
bool receiveAck(server *theServer)
{
        // no ACK came in 3 seconds, so set sentPkt array for retransmission
        if (clientTimeout(theServer)) {
                for (int i = theServer->packetsSent; i < theServer->packetsSent 
                + theServer->clientMsg->winSeq; i++) {
                        theServer->sentPkt[i] = 0;
                }
                return false;
        }
        
        // receive ACK message
        message *ackMessage = malloc(sizeof(message));
        int returnVal = recvfrom(theServer->socketFD, ackMessage, 
                sizeof(message), 0, (struct sockaddr *)&theServer->clientAddress, 
                &theServer->clientAddressLength);
        checkNegOneError(returnVal);

        // set the sentPkt array for pkt received for valid ACK
        if ((ackMessage->winSeq + 1 > theServer->packetsSent)
        && (ackMessage->winSeq <= theServer->mostRecentPkt)) {
                theServer->packetsSent = ackMessage->winSeq + 1;
                for (int i = 0; i < theServer->packetsSent; i++) {
                        theServer->sentPkt[i] = 2;
                }
        }
        return true;
}





/*****************************************************************************
*                             SENDING FUNCTIONS
******************************************************************************/


/*
 * name:      sendWindow
 * purpose:   sends the current window to the client (doesn't send packets that 
 *            are already sent that are waiting for an ACK)
 * arguments: the server instance
 * returns:   none
 * effects:   none
 */
void sendWindow(server *theServer)
{
        int windowSize = theServer->clientMsg->winSeq;
        int startPkt = theServer->packetsSent;
        int endPkt = theServer->numPackets;
        if ((startPkt + windowSize) < endPkt) {
                endPkt = startPkt + windowSize;
        }

        // determine the bytes left to send
        int bytesSent = theServer->packetsSent * 512;
        int bytesLeft = theServer->fileSize - bytesSent;

        message *dataMessage = malloc(sizeof(message));
        dataMessage->msgType = 2;
        dataMessage->winSeq = theServer->packetsSent;

        // send the packets in the window
        for (int i = startPkt; i < endPkt; i++) {
                int dataSize = 512;
                if (bytesLeft < dataSize) {
                        dataSize = bytesLeft;
                }

                sendPacket(theServer, i, dataMessage, dataSize);
                bytesLeft -= dataSize;
                theServer->mostRecentPkt = i;
        }
}


/*
 * name:      sendPacket
 * purpose:   sends a packet to the cilent if the packet was never sent or 
 *            needs to be retransmitted
 * arguments: the server instance, the packet number, the message, the number 
 *            of file bytes to transmit
 * returns:   none
 * effects:   none
 */
void sendPacket(server *theServer, int packNum, message *dataMessage, 
        int dataSize)
{
        // packet was already sent and shouldn't be retransmitted so return
        if (theServer->sentPkt[packNum] != 0) {
                dataMessage->winSeq++;
                return;
        }

        // copy the file data into the message data
        memcpy(dataMessage->data, theServer->fileContents + (packNum * 512), 
                dataSize); 

        int returnVal = sendto(theServer->socketFD, (void *)dataMessage, 
                dataSize + 2, 0, (struct sockaddr *)&theServer->clientAddress, 
                theServer->clientAddressLength);
        checkNegOneError(returnVal);

        // update the sequence number and set the sent packet to sent in array
        dataMessage->winSeq++;
        theServer->sentPkt[packNum] = 1;
}


/*
 * name:      sendErrorMessage
 * purpose:   sends an error message to the client indicating that the file 
 *            doesn't exist
 * arguments: the server instance
 * returns:   none
 * effects:   none
 */
void sendErrorMessage(server *theServer)
{
        message errorMessage;
        errorMessage.msgType = 4;

        int returnVal = sendto(theServer->socketFD, (void *)&errorMessage, 
                1, 0, (struct sockaddr *)&theServer->clientAddress, 
                theServer->clientAddressLength);
        checkNegOneError(returnVal);
}



/*****************************************************************************
*                              FILE FUNCTIONS
******************************************************************************/


/*
 * name:      getRequestedFile
 * purpose:   opens the requested file and gets its contents and size, or 
 *            returns false if the file doesn't exist
 * arguments: the server instance
 * returns:   none
 * effects:   opens and closes the requested file
 */
bool getRequestedFile(server *theServer)
{
        // try to open the file
        int fileFD = open(theServer->clientMsg->data, O_RDONLY);
        if (fileFD < 0) {
                return false;
        }

        // get the file size
        struct stat fileStat;
        int returnVal = fstat(fileFD, &fileStat);
        checkNegOneError(returnVal);
        size_t fileSize = fileStat.st_size;
        theServer->fileContents = malloc(fileSize + 1);
        checkAllocError(theServer->fileContents);

        // read the file and store contents in the buffer
        int bytesRead = read(fileFD, theServer->fileContents, fileSize);
        checkNegOneError(bytesRead);

        // set null terminator and set the file size
        theServer->fileContents[fileSize] = '\0';
        theServer->fileSize = fileSize;

        close(fileFD);
        return true;
}


/*
 * name:      setFileInformation
 * purpose:   sets the values of numPackets, packetsSent, and sentPkt array
 * arguments: the server instance
 * returns:   none
 * effects:   none
 */
void setFileInformation(server *theServer)
{
        theServer->numPackets = (theServer->fileSize / 512) + 1;
        theServer->packetsSent = 0;
        theServer->sentPkt = malloc(theServer->numPackets * sizeof(int));

        for (int i = 0; i < theServer->numPackets; i++) {
                theServer->sentPkt[i] = 0;
        }
}




/*****************************************************************************
*                              HELPER FUNCTIONS
******************************************************************************/


/*
 * name:      createSocket
 * purpose:   creates the socket for the server to receive clients on
 * arguments: the server instance
 * returns:   none
 * effects:   none
 */
void createSocket(server *theServer)
{
        struct sockaddr_in serverAddress;
        theServer->socketFD = socket(AF_INET, SOCK_DGRAM, 0);
        checkNegOneError(theServer->socketFD);

        int optVal = 1;
        setsockopt(theServer->socketFD, SOL_SOCKET, SO_REUSEADDR, 
                (const void *)&optVal, sizeof(int));

        // build the server's Internet address
        bzero((char *) &serverAddress, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddress.sin_port = htons((unsigned short)theServer->portNumber);

        // associate the parent socket with a port 
        int returnVal = bind(theServer->socketFD, 
                (struct sockaddr *)&serverAddress, sizeof(serverAddress));
        checkNegOneError(returnVal);
}


/*
 * name:      clientTimeout
 * purpose:   does a select call with timeout value of 3 seconds
 * arguments: the server instance
 * returns:   true if the client timed out, false otherwise
 * effects:   none
 */
bool clientTimeout(server *theServer)
{
        fd_set readFDS;
        struct timeval timeOut;
        timeOut.tv_sec = 3;
        timeOut.tv_usec = 0;
        FD_ZERO(&readFDS);
        FD_SET(theServer->socketFD, &readFDS);
        int returnVal = select(theServer->socketFD + 1, &readFDS, NULL, NULL, 
                &timeOut);
        checkNegOneError(returnVal);

        if (returnVal == 0) {
                return true;
        }
        return false;
}


/*
 * name:      freeClientInfo
 * purpose:   frees all the data associated with a client
 * arguments: the server instance
 * returns:   none
 * effects:   none
 */
void freeClientInfo(server *theServer)
{
        if (theServer->clientMsg != NULL) {
                free(theServer->clientMsg);
                theServer->clientMsg = NULL;
        }
        
        if (theServer->fileContents != NULL) {
                free(theServer->fileContents);
                theServer->fileContents = NULL;
        }
        theServer->fileSize = 0;
        theServer->numPackets = 0;
        theServer->packetsSent = 0;
        theServer->mostRecentPkt = -1;
        if (theServer->sentPkt != NULL) {
                free(theServer->sentPkt);
                theServer->sentPkt = NULL;
        }
}


/*
 * name:      checkRRQFields
 * purpose:   checks that the RRQ fields are within the bounds of valid values
 * arguments: the server instance
 * returns:   true if the fields are valid, false if not
 * effects:   none
 */
bool checkRRQFields(server *theServer)
{
        bool RRQValid = true;

        if ((theServer->clientMsg->msgType != 1)
        || (theServer->clientMsg->winSeq > 9)
        || (theServer->clientMsg->winSeq < 1)) {
                RRQValid = false;
        }

        return RRQValid;
}



/*
 * name:      checkNegOneError
 * purpose:   checks the return value of a function and exits the program if 
 *            it is -1
 * arguments: the return value
 * returns:   none
 * effects:   none
 */
void checkNegOneError(int returnVal)
{
        if (returnVal < 0) {
                printf("Something went wrong, program exiting\n");
                exit(EXIT_FAILURE);
        }
}


/*
 * name:      checkAllocError
 * purpose:   checks an pointer to allocated memory and exits the program if 
 *            it is NULL
 * arguments: the return value
 * returns:   none
 * effects:   none
 */
void checkAllocError(char *memAllocated)
{
        if (memAllocated == NULL) {
                printf("Memory could not be allocated, program exiting\n");
                exit(EXIT_FAILURE);
        }
}



/*
 * name:      printHeader
 * purpose:   prints out message header information
 * arguments: a pointer to the header
 * returns:   none
 * effects:   none
 */
// void printMessage(message *header) 
// {
//         printf("\n");
//         if (header->msgType == 1) {
//                 printf("RRQ\n");
//         }
//         else if (header->msgType == 2) {
//                 printf("DATA\n");
//         }
//         else if (header->msgType == 3) {
//                 printf("ACK\n");
//         }
//         else if (header->msgType == 4) {
//                 printf("ERROR\n");
//         }
//         printf("Message Header:\n");
//         printf("  msgType: %d\n", header->msgType);
//         printf("  winSize: %d\n", header->winSeq);
//         // printf("  fileName: %s\n", header->data);
//         printf("\n");
// }

