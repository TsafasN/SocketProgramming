#ifndef UTIL_SOCKETUTIL_H
#define UTIL_SOCKETUTIL_H

#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>

#define BUFFER_SIZE 255
#define CHAT_BUFFER_SIZE 1024
#define DEFAULT_PORT 2000

// Structure for accepted socket connections
struct acceptedSocket {
    int fileDescriptor;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

// Structure for chat data
struct ChatData {
    int socketFD;
    bool *running;
};

// Socket creation and setup functions
int createTCPIpv4Socket();
struct sockaddr_in* createIPv4Address(char *ip, int port);
struct acceptedSocket* acceptIncomingConnection(int serverSocketFD);

// Chat handling functions
void* receive_messages(void* arg);
void* send_messages(void* arg);
int handleIncomingData(int socketFD);

// File transfer function
int handleFileReceive(int socketFD);
int handleFileSend(int socketFD);

// Error handling
void error(const char *msg);

#endif //UTIL_SOCKETUTIL_H
