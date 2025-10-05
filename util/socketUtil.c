#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "socketUtil.h"

int createTCPIpv4Socket()
{
    return socket(AF_INET, SOCK_STREAM, 0); 
}

struct sockaddr_in* createIPv4Address(char *ip, int port)
{
    struct sockaddr_in* address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    if(strlen(ip) == 0)
    {
        address->sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    }

    return address;
}

struct acceptedSocket* acceptIncomingConnection(int serverSocketFD)
{
    struct sockaddr_in clientAddress;
    struct acceptedSocket* acceptedSocket = malloc(sizeof(struct acceptedSocket));
    
    printf("Waiting for client connections:...\n");

    int clientAddressSize = sizeof(struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, (struct sockaddr*)&clientAddress, &clientAddressSize);
    acceptedSocket->fileDescriptor = clientSocketFD;
    acceptedSocket->address = clientAddress;
    if(clientSocketFD < 0)
    {
        acceptedSocket->error = clientSocketFD;
        acceptedSocket->acceptedSuccessfully = false;
        printf("Error on Accept.\n");
    }
    else
    {
        acceptedSocket->error = 0;
        acceptedSocket->acceptedSuccessfully = true;
        printf("Accepted connection on server socket listen.\n");
        printf("Opened client address file descriptor.\n");
    }

    return acceptedSocket;
}

void* receive_messages(void* arg)
{
    struct ChatData* data = (struct ChatData*)arg;
    char buffer[CHAT_BUFFER_SIZE];

    while(*(data->running)) {
        ssize_t amountReceived = recv(data->socketFD, buffer, CHAT_BUFFER_SIZE - 1, 0);

        if(amountReceived > 0) {
            buffer[amountReceived] = 0;
            char *newline = strchr(buffer, '\n');
            if (newline) *newline = 0;
            
            printf("\nClient: %s\nYou: ", buffer);
            fflush(stdout);
        }

        if(amountReceived <= 0) {
            printf("\nClient disconnected.\n");
            *(data->running) = false;
            break;
        }
    }
    return NULL;
}

void* send_messages(void* arg)
{
    struct ChatData* data = (struct ChatData*)arg;
    char buffer[CHAT_BUFFER_SIZE];

    while(*(data->running)) {
        printf("You: ");
        fflush(stdout);
        
        if(fgets(buffer, CHAT_BUFFER_SIZE, stdin) != NULL) {
            if(strcmp(buffer, "exit\n") == 0) {
                *(data->running) = false;
                break;
            }
            
            if(send(data->socketFD, buffer, strlen(buffer), 0) < 0) {
                printf("Error sending message\n");
                *(data->running) = false;
                break;
            }
        }
    }
    return NULL;
}

int handleIncomingData(int socketFD)
{
    bool chat_running = true;
    struct ChatData chat_data = {
        .socketFD = socketFD,
        .running = &chat_running
    };

    pthread_t recv_thread, send_thread;
    
    if(pthread_create(&recv_thread, NULL, receive_messages, &chat_data) != 0) {
        printf("Error creating receive thread\n");
        return -1;
    }
    if(pthread_create(&send_thread, NULL, send_messages, &chat_data) != 0) {
        printf("Error creating send thread\n");
        return -1;
    }

    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);

    return 0;
}

int handleFileReceive(int socketFD)
{
    char buffer[BUFFER_SIZE];
    FILE *fp;
    int words = 0;

    fp = fopen("glad_received.txt", "w");
    if(fp == NULL) {
        error("Error opening file");
    }

    if(read(socketFD, &words, sizeof(int)) <= 0) {
        error("Error receiving file size");
    }

    printf("Receiving file with %d words...\n", words);

    int ch = 0;
    while(ch < words)
    {
        size_t pos = 0;
        while(1) {
            ssize_t bytesRead = read(socketFD, &buffer[pos], 1);
            if(bytesRead <= 0) {
                printf("Connection error or closed by client\n");
                fclose(fp);
                return -1;
            }
            if(buffer[pos] == '\0') break;
            pos++;
            if(pos >= BUFFER_SIZE - 1) {
                printf("Word too long!\n");
                fclose(fp);
                return -1;
            }
        }
        
        fprintf(fp, "%s ", buffer);
        ch++;
    }

    fclose(fp);
    printf("The file has been received successfully. It is saved as glad_received.txt\n");
    return 0;
}

int handleFileSend(int socketFD)
{
    FILE *fp;
    char buffer[1024];
    int words = 0;
    
    // Open file
    fp = fopen("glad.txt", "r");
    if(fp == NULL)
    {
        error("Error opening file glad.txt");
    }
    
    // Count words
    char ch;
    int is_word = 0;
    while((ch = fgetc(fp)) != EOF)
    {
        if(ch == ' ' || ch == '\n' || ch == '\t')
        {
            if(is_word)
            {
                words++;
                is_word = 0;
            }
        }
        else
        {
            is_word = 1;
        }
    }
    if(is_word) words++; // Count last word if file doesn't end with space
    
    // Send word count
    if(write(socketFD, &words, sizeof(int)) < 0)
    {
        error("Error sending word count");
    }
    
    printf("Sending file with %d words...\n", words);
    
    // Reset file pointer to beginning
    rewind(fp);
    
    // Send file word by word
    while(fscanf(fp, "%s", buffer) == 1)
    {
        if(write(socketFD, buffer, strlen(buffer) + 1) < 0)
        {
            error("Error sending file content");
        }
    }
    
    fclose(fp);
    printf("File sent successfully.\n");
    return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
