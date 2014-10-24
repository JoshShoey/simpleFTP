/*
 * File: daemon.h
 * Author: Joshua Shoebridge
 *
 * Description: This header file contains the function declarations for the 
 * daemon.c file. These functions relate to setting up the myftpd server 
 * program as a daemon and setting up signal handlers to claim child processes 
 * that have ended.
 */

#ifndef _DAEMON_H
#define _DAEMON_H

#include <signal.h> // Signal handling
#include <stdio.h> // Standard I/O stuff, perror, etc.
#include <stdlib.h> // Provides exit()
#include <sys/types.h> // Provides pid_t
#include <unistd.h> // Provides fork()

/*
 * Function: claimChildren
 * Returns: void
 * Parameters: int - signal number
 *
 * Use: This function is our SIGCHLD signal handler. Its parameter and return
 * are as such because of what sigaction expects as a signal handler. When a 
 * SIGCHLD signal is received this function will claim as many zombie processes
 * as it can before returning.
 */
void claimChildren(int);

/*
 * Function: daemonInit
 * Returns: int - status: 0 on success, -1 on failure
 * Parameters: void
 *
 * Use: This function is used to initialise the program as a daemon. It handles
 * forking off a new process and setting it as its own session leader. It also
 * sets up the signal handler for when a child process ends so they can be 
 * claimed.
 */
int daemonInit(void);

#endif
