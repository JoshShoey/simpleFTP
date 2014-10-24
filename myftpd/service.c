/*
 * File: service.c
 * Author: Joshua Shoebridge
 *
 * Description: This file contains the function definitions relating to the 
 * serving of a client that has connected to the myftpd server.
 */

#include "service.h"

void serveClient(int socketID)
{
	int bytesRead, check, numFiles, i, openFile, bytesRemain;
	int int2bytes;
	long int4bytes;
	char commandSelection, tempChar;
	char inputBuffer[BUFFER_SIZE];
	char fileBuffer[BUFFER_SIZE];
	char tempString[256];
	struct dirent **fileList;
	struct stat fileInfo;

	while (1)
	{
		// Read first opcode
		bytesRead = read(socketID, &commandSelection, sizeof(char));
		// If connection has broken down then exit
		if (bytesRead <= 0)
		{
			break;
		}
		// Check for command given
		if (commandSelection == 'A') // cd
		{
			// read in directory name length
			bytesRead = read(socketID, &int2bytes, 
				sizeof(int2bytes));
			// convert back to host byte order
			int2bytes = ntohs(int2bytes);
			// read in directory name
			bytesRead = read(socketID, inputBuffer, 
				(sizeof(char) * int2bytes));
			// change the pwd
			check = chdir(inputBuffer);
			// Send back 'A' opcode for response
			tempChar = 'A';
			write(socketID, &tempChar, sizeof(char));

			if (check == -1) // If directory change fails
			{
				if (errno == ENOENT) // Dir does not exist
				{
					tempChar = '1';
				} else if (errno == EACCES) // Access denied
				{
					tempChar = '2';
				} else // Other errors
				{
					tempChar = '3';
				}	
			} else
			{
				tempChar = '0';
			}

			// send back status of directory change
			write(socketID, &tempChar, sizeof(tempChar));
		} else if (commandSelection == 'B') // dir
		{
			// Send back 'B' for response
			tempChar = 'B';
			write(socketID, &tempChar, sizeof(char));
			// Scan directory for list of files
			numFiles = scandir(".", &fileList, NULL, alphasort);
			if (numFiles == 0)
			{
				perror("myftpd:scandir");
			}
			// Send number of files and folders
			numFiles = htons(numFiles);
			write(socketID, &numFiles, sizeof(numFiles));
			// Iterate through files and send
			numFiles = ntohs(numFiles);
			for (i = 0; i < numFiles; ++i)
			{
				// Figure out file type
				if (fileList[i]->d_type == DT_REG)
				{
					tempChar = 'f';
				} else if (fileList[i]->d_type == DT_DIR)
				{
					tempChar = 'd';
				} else
				{
					tempChar = 'o';
				}
				// Send file type
				write(socketID, &tempChar, sizeof(char));
				// Get filename length and send
				int2bytes = strlen(fileList[i]->d_name) + 1;
				int2bytes = htons(int2bytes);
				write(socketID, &int2bytes, sizeof(int2bytes));
				// Send filename
				write(socketID, fileList[i]->d_name, 
					ntohs(int2bytes));
				free(fileList[i]);
			}

			free(fileList);
		} else if (commandSelection == 'C') // get
		{
			// Get filename length
			read(socketID, &int2bytes, sizeof(int2bytes));
			// Get filename
			read(socketID, inputBuffer, 
				(sizeof(char) * ntohs(int2bytes)));
			// Open file to send
			openFile = open(inputBuffer, O_RDONLY);
			// Send command 'C' opcode
			tempChar = 'C';
			write(socketID, &tempChar, sizeof(tempChar));
			// If error with file open send error code and continue
			if (openFile == -1)
			{
				if(errno == ENOENT)
				{
					tempChar = '1';
				} else if (errno == EACCES)
				{
					tempChar = '2';
				} else
				{
					tempChar = '3';
				}

				write(socketID, &tempChar, sizeof(tempChar));
				continue;
			}
			// If no error, send success opcode
			tempChar = '0';
			write(socketID, &tempChar, sizeof(tempChar));
			// Get file info
			stat(inputBuffer, &fileInfo);
			int4bytes = fileInfo.st_size;
			// Send file size
			int4bytes = htonl(int4bytes);
			write(socketID, &int4bytes, sizeof(int4bytes));
			int4bytes = ntohl(int4bytes);
			bytesRemain = int4bytes;
			// Loop through and send file
			while(bytesRemain)
			{
				if(bytesRemain < BUFFER_SIZE)
				{
					bytesRead = read(openFile, fileBuffer, 
						(sizeof(char) * bytesRemain));
				} else
				{
					bytesRead = read(openFile, fileBuffer,
						(sizeof(char) * BUFFER_SIZE));
				}
				write(socketID, fileBuffer, bytesRead);
				bytesRemain = bytesRemain - bytesRead;
			}
			// Close the opened file
			close(openFile);
		} else if (commandSelection == 'D') // put
		{
			// Get filename length
			read(socketID, &int2bytes, sizeof(int2bytes));
			int2bytes = ntohs(int2bytes);
			// Get filename
			read(socketID, inputBuffer, 
				(sizeof(char) * int2bytes));
			// Send back opcode
			tempChar = 'D';
			write(socketID, &tempChar, sizeof(tempChar));
			// Open local file for writing
			openFile = open(inputBuffer, 
				O_WRONLY | O_CREAT | O_APPEND | O_EXCL, 0644);
			if (openFile == -1)
			{
				if (errno == EEXIST)
				{
					tempChar = '1';
				} else if (errno == EACCES)
				{
					tempChar = '2';
				} else
				{
					tempChar = '3';
				}

				// Send status code
				write(socketID, &tempChar, sizeof(tempChar));
				continue;
			}
			// If success set status code and send
			tempChar = '0';
			write(socketID, &tempChar, sizeof(tempChar));
			// Receive next opcode
			read(socketID, &tempChar, sizeof(tempChar));
			// Receive file size from client
			read(socketID, &int4bytes, sizeof(int4bytes));
			int4bytes = ntohl(int4bytes);
			bytesRemain = int4bytes;
			// Receive file from client
			while (bytesRemain)
			{
				if (bytesRemain < BUFFER_SIZE)
				{
					bytesRead = read(socketID, fileBuffer,
						(sizeof(char) * bytesRemain));
				} else
				{
					bytesRead = read(socketID, fileBuffer,
						(sizeof(char) * BUFFER_SIZE));
				}
				write(openFile, fileBuffer, bytesRead);
				bytesRemain = bytesRemain - bytesRead;
			}
			// Close open file
			close(openFile);
		} else if (commandSelection == 'E') // pwd
		{
			// Set tempChar to 'E' for server response
			tempChar = 'E';
			// Get pwd
			getcwd(tempString, 256);
			// get pwd length
			int2bytes = strlen(tempString) + 1;
			int2bytes = htons(int2bytes);
			// Send opcode
			write(socketID, &tempChar, sizeof(tempChar));
			// Send pwd length
			write(socketID, &int2bytes, sizeof(int2bytes));
			// Send the pwd
			write(socketID, tempString, ntohs(int2bytes));
		} else // Just in case, shouldn't get here
		{

		}
	}
}
