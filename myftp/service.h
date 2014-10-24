/*
 * File: service.h
 * Author: Joshua SHoebridge
 *
 * Description: This header file contains the function declarations for the
 * service.c file. These functions relate to the frontend provided by the myftp
 * client program.
 */

#ifndef _SERVICE_H
#define _SERVICE_H

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define IN_BUF_SIZE 256
#define NET_BUF_SIZE 512
#define TOK_DELIM " \n" // Delimiter for first command, uses spaces
#define TOK_DELIM_NS "\n" // Delimiter for command arguments, no spaces

/*
 * Function: prompt
 * Returns: void
 * Parameters: int - open socket
 *
 * Use: This is the main function for providing functionality of the client. It
 * provides a 'prompt' and translates what is entered in the prompt as commands
 * that need to be run.
 */
void prompt(int);

#endif
