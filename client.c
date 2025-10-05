/*
filename server_ipaddress portno

argv[0] filename
argv[1] server_ipaddress
argv[2] portno

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "util/socketUtil.h"

#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>

int handleChat(int socketFD)
{
    bool chat_running = true;
    struct ChatData chat_data = {
        .socketFD = socketFD,
        .running = &chat_running
    };

    printf("Chat mode. Type your messages (type 'exit' to quit):\n");
    
    pthread_t recv_thread, send_thread;
    
    // Create threads for sending and receiving
    if(pthread_create(&recv_thread, NULL, receive_messages, &chat_data) != 0) {
        printf("Error creating receive thread\n");
        return -1;
    }
    if(pthread_create(&send_thread, NULL, send_messages, &chat_data) != 0) {
        printf("Error creating send thread\n");
        return -1;
    }

    // Wait for both threads to finish
    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);

    return 0;
}

int main(int argc, char *argv[])
{
    int portno = 2000;
    char *host = "127.0.0.1";
    
    // Allow command line arguments for host and port
    if(argc > 1) host = argv[1];
    if(argc > 2) portno = atoi(argv[2]);

    //Create file descriptor for client socket
    int socketFD = createTCPIpv4Socket();
    if(socketFD < 0)
    {
        error("Error opening socket.");
    }

    struct sockaddr_in *address = createIPv4Address(host, portno);

    //Connect using the client socket file descriptor
    int resultConnect = connect(socketFD, (struct sockaddr *) address, sizeof(*address));
    if(resultConnect < 0)
    {
        error("Connection Failed");
    }
    printf("Connected to server at %s:%d\n", host, portno);

    // Choose mode
    printf("Choose mode:\n");
    printf("1. Chat mode\n");
    printf("2. File transfer mode\n");
    printf("3. Exit\n");
    
    char choice;
    scanf(" %c", &choice);  // Space before %c to consume any whitespace
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    // Send mode to server first
    if(send(socketFD, &choice, 1, 0) < 0) {
        error("Error sending mode to server");
    }
    
    if(choice == '1')
    {
        printf("Chat Mode Selected.\n");
        handleChat(socketFD);
    }
    else if(choice == '2')
    {
        printf("File Transfer Mode Selected.\n");
        handleFileSend(socketFD);
    }
    else if(choice == '3')
    {
        printf("Exiting.\n");
    }
    else
    {
        printf("Invalid choice.\n");
    }

    close(socketFD);
    free(address);
    return 0;
}

