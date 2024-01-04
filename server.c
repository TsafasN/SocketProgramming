#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// #include "util/socketUtil.h"
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>

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

struct acceptedSocket
{
	int fileDescriptor;
	struct sockaddr_in address;
	int error;
	bool acceptedSuccessfully;
};

//Await a connection on server socket file descriptor
//When a connection arrives, open a new socket to communicate with it
struct acceptedSocket* acceptIncomingConnection(int serverSocketFD);

int handleIncomingData(int socketFD);

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

	//Create file descriptor for server socket
	int serverSocketFD = createTCPIpv4Socket();
	if(serverSocketFD < 0)
	{
		error("Error opening socket.");
	}

	//Bind using the server socket file descriptor
	struct sockaddr_in *serverAddress = createIPv4Address("", 2000);
	int resultBind = bind(serverSocketFD, (struct sockaddr *) serverAddress, sizeof(*serverAddress));
	if(resultBind < 0)
	{
		error("Binding Failed.");
	}
	printf("socket was bound successfully.\n");

	//Prepare to accept connections on server socket file descriptor
	int listenResult = listen(serverSocketFD, 10);

	//Wait a connection on server socket file descriptor
	struct acceptedSocket* clientSocket = acceptIncomingConnection(serverSocketFD);

	handleIncomingData(clientSocket->fileDescriptor);

	close(clientSocket->fileDescriptor);
	shutdown(serverSocketFD, SHUT_RDWR);

	return 0;
}


struct acceptedSocket* acceptIncomingConnection(int serverSocketFD)
{
	printf("Waiting for client connections:...\n");

	struct sockaddr_in clientAddress;
	int clientAddressSize = sizeof(struct sockaddr_in);
	int clientSocketFD = accept(serverSocketFD,  (struct sockaddr *) &clientAddress, &clientAddressSize);

	struct acceptedSocket* acceptedSocket = malloc(sizeof(struct acceptedSocket));
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

int handleIncomingData(int socketFD)
{
	char buffer[1024];

	while(true)
	{
		ssize_t amountReceived = recv(socketFD, buffer, 1024, 0);

		if(amountReceived > 0)
		{
			buffer[amountReceived] = 0;
			printf("Response was %s\n", buffer);
		}

		if(amountReceived == 0)
		{
			break;		
		}
	}
}
