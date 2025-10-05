#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "util/socketUtil.h"

int main(int argc, char *argv[])
{
    int portno = DEFAULT_PORT;
    
    // Allow port override from command line
    if(argc > 1) {
        portno = atoi(argv[1]);
    }

    //Create file descriptor for server socket
    int serverSocketFD = createTCPIpv4Socket();
    if(serverSocketFD < 0)
    {
        error("Error opening socket.");
    }

    //Bind using the server socket file descriptor
    struct sockaddr_in *serverAddress = createIPv4Address("", portno);
    int resultBind = bind(serverSocketFD, (struct sockaddr *) serverAddress, sizeof(*serverAddress));
    if(resultBind < 0)
    {
        error("Binding Failed.");
    }
    printf("Socket was bound successfully on port %d.\n", portno);

    //Prepare to accept connections on server socket file descriptor
    int listenResult = listen(serverSocketFD, 10);

    while(1) {
        printf("\nWaiting for new connection...\n");
        
        //Wait a connection on server socket file descriptor
        struct acceptedSocket* clientSocket = acceptIncomingConnection(serverSocketFD);
        
        if(!clientSocket->acceptedSuccessfully) {
            free(clientSocket);
            continue;
        }

        // Receive mode choice from client
        char mode;
        ssize_t recv_size = recv(clientSocket->fileDescriptor, &mode, 1, MSG_WAITALL);

        if(recv_size <= 0) {
            printf("Error receiving mode from client\n");
            close(clientSocket->fileDescriptor);
            free(clientSocket);
            continue;
        }

        printf("Received mode '%c' from client\n", mode);

        switch(mode) {
            case '1':
                printf("Client selected Chat Mode.\n");
                handleIncomingData(clientSocket->fileDescriptor);
                break;
            case '2':
                printf("Client selected File Transfer Mode.\n");
                handleFileReceive(clientSocket->fileDescriptor);
                break;
            default:
                printf("Invalid mode received from client.\n");
        }

        close(clientSocket->fileDescriptor);
        free(clientSocket);
    }

    shutdown(serverSocketFD, SHUT_RDWR);
    close(serverSocketFD);
    free(serverAddress);

    return 0;
}
