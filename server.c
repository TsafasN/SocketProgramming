#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

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
	int sockfd, newsockfd, portno, n;
	char buffer[255];
	struct sockaddr_in serv_addr, cli_addr;
	
	//Check call arguments
	if(argc < 2)
	{
		fprintf(stderr, "Port number not provided, Program terminated /r/n");
		exit(1);
	}

	//Create file descriptor for server socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		error("Error opening socket.");
	}

	//Port number, get from arguments
	portno = atoi(argv[1]);

	//Initialize sockaddr structure
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	//Bind using the server socket file descriptor
	if( bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("Binding Failed.");

	//Prepare to accept connections on server socket file descriptor
	listen(sockfd, 5);

	//Await a connection on server socket file descriptor
	//When a connection arrives, open a new socket to communicate with it
	socklen_t clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if(newsockfd < 0)
		error("Error on Accept.");

	FILE *fp;

	int ch = 0;
	fp = fopen("glad_received.txt", "w");

	int words;
	read(newsockfd, &words, sizeof(int));

	while(ch != words)
	{
		read(newsockfd, buffer, 255);
		fprintf(fp, "%s ", buffer);
		ch++;
	}

	printf("The file has been received successfully. It is saved by the name glad_received.txt.");

	close(newsockfd);
	close(sockfd);

	return 0;
}

