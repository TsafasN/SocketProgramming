#include <stdio.h>
#include <stdlib.h>
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

	struct sockaddr_in *serverAddress = createIPv4Address("", 2000);
	//Bind using the server socket file descriptor
	int resultBind = bind(serverSocketFD, (struct sockaddr *) serverAddress, sizeof(*serverAddress));
	if(resultBind < 0)
	{
		error("Binding Failed.");
	}
	printf("socket was bound successfully.\n");

	//Prepare to accept connections on server socket file descriptor
	int listenResult = listen(serverSocketFD, 10);

	printf("Waiting for client connections:...\n");
	//Await a connection on server socket file descriptor
	//When a connection arrives, open a new socket to communicate with it
	struct sockaddr_in clientAddress;
	int clientAddressSize = sizeof(struct sockaddr_in);
	int clientAddressFD = accept(serverSocketFD,  (struct sockaddr *) &clientAddress, &clientAddressSize);
	if(clientAddressFD < 0)
	{
		error("Error on Accept.");
	}
	printf("Accepted connection on server socket listen.\n");
	printf("Opened client address file descriptor.\n");

	char buffer[255];
	FILE *fp;

	int ch = 0;
	fp = fopen("glad_received.txt", "w");

	int words;
	read(clientAddressFD, &words, sizeof(int));

	while(ch != words)
	{
		read(clientAddressFD, buffer, 255);
		fprintf(fp, "%s ", buffer);
		ch++;
	}

	printf("The file has been received successfully. It is saved by the name glad_received.txt.\n");

	close(clientAddressFD);
	close(serverSocketFD);

	return 0;
}

