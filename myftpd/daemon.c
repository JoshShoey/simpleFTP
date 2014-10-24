/*
 * File: daemon.c
 * Author: Joshua Shoebridge
 *
 * Description: This file contains the function definitions relating to setting
 * up the myftpd server program as a daemon. It is also where the interrupt 
 * handlers are set up for claiming child processes that have ended.
 */

#include "daemon.h"

void claimChildren(int signo)
{
	pid_t pid;

	do {
		pid = waitpid(0, (int *)0, WNOHANG);
	} while (pid > 0);
}

int daemonInit(void)
{
	pid_t pid;
	struct sigaction actChild;

	if ((pid = fork()) < 0) // If error, return -1
	{
		return (-1);
	} else if (pid != 0) // If success, parent exits
	{
		exit(0);
	}

	setsid(); // Makes child its own session leader
	umask(2); // Set umask to my system preferred

	actChild.sa_flags = SA_NOCLDSTOP; // Don't signal when child stops
	actChild.sa_handler = claimChildren; // Set our handler function
	sigemptyset(&(actChild.sa_mask)); // Don't block other signals

	// Set our signal handler for SIGCHLD, exit if fail
	if ((sigaction(SIGCHLD, &actChild, NULL)) != 0)
	{
		perror("sigaction");
		exit(1);
	}

	return (0);
}
