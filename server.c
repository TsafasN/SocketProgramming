#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <pthread.h>

#define CHAT_BUFFER_SIZE 1024

#define BUFFER_SIZE 255
#define DEFAULT_PORT 2000

struct acceptedSocket
{
    int fileDescriptor;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

struct sockaddr_in* createIPv4Address(char *ip, int port);
struct acceptedSocket* acceptIncomingConnection(int serverSocketFD);
int createTCPIpv4Socket();
int handleIncomingData(int socketFD);
int handleFileTransfer(int socketFD);

/*
 * Print error message and exit.
 * @param[in] msg The message to print to console.
 */
void error(const char *msg)
{
    perror(msg);
    exit(1);        
}

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

		// char buffer[BUFFER_SIZE];

		// while(true)
		// {
		// 	ssize_t amountReceived = recv(socketFD, buffer, BUFFER_SIZE - 1, 0);

		// 	if(amountReceived > 0)
		// 	{
		// 		buffer[amountReceived] = 0;
		// 		printf("Response was %s\n", buffer);
		// 	}

		// 	if(amountReceived == 0)
		// 	{
		// 		printf("Client disconnected.\n");
		// 		break;        
		// 	}
		// }

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
                handleFileTransfer(clientSocket->fileDescriptor);
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
            
            printf("\nClient: %s\nYou: ", buffer); // Add prompt after message
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

int handleIncomingData(int socketFD)
{
    bool chat_running = true;
    struct ChatData chat_data = {
        .socketFD = socketFD,
        .running = &chat_running
    };

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
    char buffer[BUFFER_SIZE];
    FILE *fp;
    int words = 0;

    fp = fopen("glad_received.txt", "w");
    if(fp == NULL) {
        error("Error opening file");
    }

    // Receive number of words
    if(read(socketFD, &words, sizeof(int)) <= 0) {
        error("Error receiving file size");
    }

    printf("Receiving file with %d words...\n", words);

    int ch = 0;
    size_t total_bytes = 0;
    while(ch < words)
    {
        // Read until we find a null terminator
        size_t pos = 0;
        while(1) {
            ssize_t bytesRead = read(socketFD, &buffer[pos], 1);
            if(bytesRead <= 0) {
                printf("Connection error or closed by client\n");
                fclose(fp);
                return -1;
            }
            if(buffer[pos] == '\0') break;  // Found end of word
            pos++;
            if(pos >= BUFFER_SIZE - 1) {
                printf("Word too long!\n");
                fclose(fp);
                return -1;
            }
        }
        
        // Write the word to file
        fprintf(fp, "%s ", buffer);
        ch++;
    }

    fclose(fp);
    printf("The file has been received successfully. It is saved as glad_received.txt\n");
    return 0;
}
