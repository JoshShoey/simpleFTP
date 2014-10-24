/*
 * File: service.h
 * Author: Joshua Shoebridge
 *
 * Description: This header file contains the function declarations for the
 * service.c file. These functions relate to serving a client that connects to
 * the server.
 */

#ifndef _SERVICE_H
#define _SERVICE_H

#include <dirent.h>
#include <errno.h> // Provides errno, etc.
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> // Provides read(), write(), etc.

#define BUFFER_SIZE 512

/*
 * Function: serveClient
 * Returns: void
 * Parameters: int - socket to serve the client on
 *
 * Use: This function provides the start point for serving a client that 
 * connects to the server. From here the client is served with the relevant
 * functions.
 */
void serveClient(int);

#endif
