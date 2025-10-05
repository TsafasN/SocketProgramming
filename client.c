/*
filename server_ipaddress portno

argv[0] filename
argv[1] server_ipaddress
argv[2] portno

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>

#define CHAT_BUFFER_SIZE 1024

struct sockaddr_in* createIPv4Address(char *ip, int port);

int createTCPIpv4Socket();

int createTCPIpv4Socket()
{
    int returnVal = 0;

    returnVal = socket(AF_INET, SOCK_STREAM, 0); 
    
    return returnVal;
}

struct sockaddr_in* createIPv4Address(char *ip, int port)
{
    struct sockaddr_in  *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    if(strlen(ip) ==0)
    {
        address->sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    }

    return address;
}

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

// Structure to pass data to threads
struct ChatData {
    int socketFD;
    bool *running;
};

// Thread function to handle receiving messages
void* receive_messages(void* arg) {
    struct ChatData* data = (struct ChatData*)arg;
    char buffer[CHAT_BUFFER_SIZE];

    while(*(data->running)) {
        ssize_t amountReceived = recv(data->socketFD, buffer, CHAT_BUFFER_SIZE - 1, 0);

        if(amountReceived > 0) {
            buffer[amountReceived] = 0;
            // Remove newline if it exists
            char *newline = strchr(buffer, '\n');
            if (newline) *newline = 0;
            
            printf("\nServer: %s\nYou: ", buffer); // Add prompt after message
            fflush(stdout);
        }

        if(amountReceived <= 0) {
            printf("\nServer disconnected.\n");
            *(data->running) = false;
            break;
        }
    }
    return NULL;
}

// Thread function to handle sending messages
void* send_messages(void* arg) {
    struct ChatData* data = (struct ChatData*)arg;
    char buffer[CHAT_BUFFER_SIZE];

    while(*(data->running)) {
        printf("You: ");
        fflush(stdout);
        
        if(fgets(buffer, CHAT_BUFFER_SIZE, stdin) != NULL) {
            // Check for exit command
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

int handleFileTransfer(int socketFD)
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
        handleFileTransfer(socketFD);
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

