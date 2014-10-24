/*
 * File: myftp.c
 * Author: Joshua Shoebridge
 *
 * Description: This is the main implementation file for the myftp client 
 * program.
 */

#include <arpa/inet.h>
#include <netdb.h> // Provides hostent
#include <netinet/in.h> // Provides sockaddr_in, htons(), htonl(), etc.
#include <stdio.h> // Standard I/O
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // Provides sockets, gethostbyname(), etc.
#include <sys/types.h>
#include <unistd.h>

#include "service.h"

#define LOCALHOST 2130706433 // 127.0.0.1
#define SERV_PORT 40355 // My 'well known' port number

/*
 * Function: addrTypeCheck
 * Returns: int - 0 if IP address, 1 if hostname
 * Parameters: char * - string containing IP address or hostname
 *
 * Use: This is just a quick check to figure out if the parameter provided
 * as a command line argument is an IP address or a hostname.
 */
int addrTypeCheck(char *address)
{
	int i = 0;
	int strLength = strlen(address);

	for (i = 0; i < strLength; ++i)
	{
		if ((address[i] < 48) || (address[i] > 57))
		{
			if (address[i] != 46)
			{
				return (1);
			}
		}
	}

	return (0);
}

int main(int argc, char *argv[])
{
	char hostname[100];
	struct sockaddr_in servAddr;
	struct in_addr netIP; // Used to convert input string to IP
	struct hostent *servInfo; // Used to get IP from hostname
	int sock0;

	// Initialise servAddr to zero. Using memset for POSIX compliance
	memset((void *)&servAddr, 0, sizeof(servAddr));

	if (argc == 1) // No IP or hostname given, use localhost
	{
		printf("No arguments. Using localhost\n");
		servAddr.sin_addr.s_addr = htonl(LOCALHOST);
	} else if (argc == 2) // IP or hostname given
	{
		if (addrTypeCheck(argv[1]) == 0) // If IP address given
		{
			if (inet_aton(argv[1], &netIP) == 0)
			{
				perror("myftp:address");
				exit(1);
			}
			servAddr.sin_addr.s_addr = netIP.s_addr; // If valid
		} else // If hostname given
		{
			if ((servInfo = gethostbyname(argv[1])) == NULL)
			{
				perror("myftp:host");
				exit(1);
			}
			servAddr.sin_addr.s_addr = 
				*(long *)servInfo->h_addr_list[0];
		}
	} else // Too many arguments given
	{
		printf("Too many arguments. Use:\n");
		printf("%s [ IP Address | Hostname ]\n", argv[0]);
		exit(1);
	}

	// Set up network address for server
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERV_PORT);

	// Set up socket and connect to server
	sock0 = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(sock0, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
	{
		perror("myftp:connect");
		exit(1);
	}

	prompt(sock0);

	exit(0);
}
