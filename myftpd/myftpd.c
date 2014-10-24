/*
 * File: myftpd.c
 * Author: Joshua Shoebridge
 *
 * Description: This is the main file for the myftpd server program. The myftpd
 *              server is used to provide a simple file transfer protocol as 
 *              defined in the ProtocolSpecification file.
 */

#include <errno.h> // Provides error code macros and errno
#include <netinet/in.h> // Provides sockaddr_in, htons(), htonl(), etc.
#include <stdio.h> // Standard I/O, perror(),  misc. stuff
#include <string.h> // String functions, memset()
#include <sys/socket.h> // Socket stuff
#include <sys/types.h> // Provides pid_t and other data types
#include <unistd.h> // Provides fork(), read(), write(), etc.

#include "daemon.h" // My daemon functionality
#include "service.h" // Provides a service to the client

#define SERV_PORT 40355 // My server's 'well known' port number
#define BUF_SIZE 512

int main(int argc, char *argv[])
{
	int sock0, sock0con, cliAddrLength;
	struct sockaddr_in servAddr, cliAddr;
	pid_t pid;

	// Set up initial 'working directory'
	if (argc == 1)
	{
		printf("No directory specified. Using current\n");
	} else if (argc == 2)
	{
		if (chdir(argv[1]) < 0)
		{
			printf("Directory not valid. Exiting...\n");
			exit(1);
		} else
		{
			printf("Using '%s' as initial directory\n", argv[1]);
		}
	} else
	{
		printf("Too many arguments specified. Use:\n");
		printf("%s [ InitialDirectory ]\n", argv[0]);
		exit(1);
	}

	printf("*** Starting daemon on port %d ***\n", SERV_PORT);
	daemonInit(); // Become a daemon

	// Set up the listen socket parameters
	if ((sock0 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("myftpd:socket");
		exit(1);
	}

	// Set up server socket network connection
	// memset instead of bzero for POSIX compliance
	memset((void *)&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET; // Set address family to TCP/IP
	servAddr.sin_port = htons(SERV_PORT); // Set port number, net byte ord
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Use any IP address

	// Bind our address to the socket
	if (bind(sock0, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
	{
		perror("server:bind");
		exit(1);
	}

	// Start listening on the socket for new connections
	listen(sock0, 5);

	while (1)
	{
		// Wait for a client to connect
		cliAddrLength = sizeof(cliAddr);
		sock0con = accept(sock0, (struct sockaddr *)&cliAddr,
			&cliAddrLength);
		if (sock0con < 0) // If error with connection accept
		{
			if (errno == EINTR) // If error was interrupt, continue
			{
				continue;
			}
			perror("myftpd:accept"); // Otherwise exit
			exit(1);
		}
		// Once we get a connection, fork a new process to handle
		pid = fork();
		if (pid < 0) // If error on fork
		{
			perror("myftpd:fork");
			exit(1);
		} else if (pid > 0) // If parent, close sock0con
		{
			close(sock0con);
			continue;
		}

		// Child gets to here
		close(sock0); // Make child stop listening for connections

		// Serve the client connection here
		serveClient(sock0con);

		exit(0);
	}
}
