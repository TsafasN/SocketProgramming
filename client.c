/*
filename server_ipaddress portno

argv[0] filename
argv[1] server_ipaddress
argv[2] portno

*/

#include <stdio.h>
#include <stdlib.h>
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
	
	char buffer[256];
	bzero(buffer, 255);

	FILE *f;
	int words = 0;

	char c;

	f = fopen("glad.txt", "r");
	while((c = getc(f)) != EOF)
	{
		fscanf(f, "%s", buffer);
		if(isspace(c) || c=='\t')
			words++;
	}
	
	write(socketFD, &words, sizeof(int));
	rewind(f);

	char ch;
	while(ch != EOF)
	{
		fscanf(f, "%s", buffer);
		write(socketFD, buffer, 255);
		ch = fgetc(f);
	}

	printf("The file has been successfully sent. Thank you.");

	close(socketFD);
	return 0;
}

