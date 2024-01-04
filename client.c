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

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{

	//Create file descriptor for client socket
	int socketFD = createTCPIpv4Socket();
	if(socketFD < 0)
	{
		error("Error opening socket.");
	}

	struct sockaddr_in *address = createIPv4Address("127.0.0.1", 2000);

	//Connect using the client socket file descriptor
	int resultConnect = connect(socketFD, (struct sockaddr *) address, sizeof(*address));
	if(resultConnect < 0)
	{
		error("Connection Failed");
	}
	
	char *line = NULL;
	size_t lineSize = 0;
	printf("type and we will send (type exit)...\n");
	
	while(true)
	{
		printf("\n");

		ssize_t charCount = getline(&line, &lineSize, stdin);

		if (charCount > 0)
		{
			if(strcmp(line,"exit\n") == 0)
			{
				break;
			}

			ssize_t amountWasSent = send(socketFD, line, charCount, 0);
		}
	}

	close(socketFD);
	return 0;
}

