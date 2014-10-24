/*
 * File: service.c
 * Author: Joshua Shoebridge
 *
 * Description: This file contains the function definitions relating to the
 * frontend the myftp client program provides to the user.
 */

#include "service.h"

void prompt(int socketID)
{
	char inputBuffer[IN_BUF_SIZE];
	char *tempToken;
	char tempString[256];
	char tempChar;
	char fileBuffer[NET_BUF_SIZE];
	int check, numFiles, i, fileNameLen, openFile;
	int int2bytes;
	long int4bytes, bytesRead, bytesRemain;
	struct dirent **fileList;
	struct stat fileInfo;

	do {
		printf(">");

		// Get user input into buffer
		fgets(inputBuffer, IN_BUF_SIZE, stdin);
		if (strlen(inputBuffer) == 1)
		{
			continue;
		} else if ((inputBuffer[strlen(inputBuffer) - 1]) == '\n')
		{
			inputBuffer[strlen(inputBuffer) - 1] = '\0';
		}

		if (strcmp("quit", inputBuffer) == 0)
		{
			printf("Goodbye\n");
			exit(1);
		} else
		{
			// Tokenise input to get first parameter
			tempToken = strtok(inputBuffer, TOK_DELIM);

			// Figure out what the command is here
			if (strcmp("cd", tempToken) == 0)
			{
				// cd command stuff here
				// Get dir to change to
				tempToken = strtok(NULL, TOK_DELIM);
				if (tempToken == NULL)
				{
					printf("No directory specified\n");
					continue;
				}
				int2bytes = strlen(tempToken) + 1;
				int2bytes = htons(int2bytes);
				// Set opcode in tempChar
				tempChar = 'A';
				// Send opcode
				write(socketID, &tempChar, sizeof(tempChar));
				// Send length of dir name
				write(socketID, &int2bytes, sizeof(int2bytes));
				// Send dir name
				write(socketID, tempToken, 
					(sizeof(char) * (ntohs(int2bytes))));
				// Get back command response
				read(socketID, &tempChar, sizeof(char));
				// Get back status of dir change
				read(socketID, &tempChar, sizeof(char));

				if (tempChar == '1')
				{
					printf("Directory '%s' does not exist"
						"\n", tempToken);
				} else if (tempChar == '2')
				{
					printf("Access denied\n");
				} else if (tempChar == '3')
				{
					printf("Unable to change directory\n");
				}

			} else if (strcmp("dir", tempToken) == 0)
			{
				// dir command stuff here
				// Set opcode in tempChar
				tempChar = 'B';
				// Send opcode
				write(socketID, &tempChar, sizeof(tempChar));
				// Get back command response
				read(socketID, &tempChar, sizeof(tempChar));
				// Get back number of files and folders
				read(socketID, &int2bytes, sizeof(int2bytes));
				int2bytes = ntohs(int2bytes);
				// Get back file info and print
				for (i = 0; i < int2bytes; ++i)
				{
					// Get file type and print
					read(socketID, &tempChar, 
						sizeof(tempChar));
					printf("%c ", tempChar);
					// Get filename length
					read(socketID, &fileNameLen,
						sizeof(fileNameLen));
					fileNameLen = ntohs(fileNameLen);
					// Get filename and print
					read(socketID, tempString,
						(sizeof(char) * fileNameLen));
					printf("%s\n", tempString);
				}
			} else if (strcmp("get", tempToken) == 0)
			{
				// get command stuff here
				// Set opcode 'C' in tempChar
				tempChar = 'C';
				// Get the filename
				tempToken = strtok(NULL, TOK_DELIM_NS);
				if (tempToken == NULL)
				{
					printf("No file name specified\n");
					continue;
				}
				int2bytes = strlen(tempToken) + 1;
				int2bytes = htons(int2bytes);
				// Set up local file for writing
				openFile = open(tempToken, 
					O_WRONLY | O_CREAT | O_APPEND | O_EXCL
					, 0644);
				if (openFile == -1)
				{
					if (errno == EEXIST)
					{
						printf("File already exists "
							"in client folder\n");
					} else
					{
						printf("Could not create file "
							"on client\n");
					}
					continue;
				}
				// Send opcode
				write(socketID, &tempChar, sizeof(tempChar));
				// Send filename length
				write(socketID, &int2bytes, sizeof(int2bytes));
				// Send filename
				write(socketID, tempToken, 
					(sizeof(char) * ntohs(int2bytes)));
				// Get back opcode
				read(socketID, &tempChar, sizeof(tempChar));
				// Get back status code
				read(socketID, &tempChar, sizeof(tempChar));
				if (tempChar == '0') // Can send file
				{
					// Get file size
					read(socketID, &int4bytes, 
						sizeof(int4bytes));
					int4bytes = ntohl(int4bytes);
					bytesRemain = int4bytes;
					// Loop through and get file
					while (bytesRemain)
					{
						if (bytesRemain < NET_BUF_SIZE)
						{
							bytesRead = 
								read(socketID, 
								fileBuffer, 
								(sizeof(char) 
								* bytesRemain));
						} else
						{
							bytesRead = 
								read(socketID,
								fileBuffer,
								(sizeof(char)
								* 
								NET_BUF_SIZE));
						}
						write(openFile, fileBuffer,
							bytesRead);
						bytesRemain = bytesRemain - 
							bytesRead;
					}
				} else if (tempChar == '1') // File not found
				{
					printf("File not found\n");
				} else if (tempChar == '2') // Denied access
				{
					printf("Access denied\n");
				} else if (tempChar == '3') // Other error
				{
					printf("Could not get file\n");
				} else
				{
					//Shouldn't get here
					printf("Something went wrong...\n");
				}
				// Close the opened file
				close(openFile);
			} else if (strcmp("put", tempToken) == 0)
			{
				// put command stuff here
				// Set opcode in tempChar
				tempChar = 'D';
				// Get the filename
				tempToken = strtok(NULL, TOK_DELIM_NS);
				if(tempToken == NULL)
				{
					printf("No file name specified\n");
					continue;
				}
				int2bytes = strlen(tempToken) + 1;
				int2bytes = htons(int2bytes);
				// Open file for sending
				openFile = open(tempToken, O_RDONLY);
				if (openFile == -1)
				{
					if (errno == ENOENT)
					{
						printf("Could not find file "
							"to send\n");
					} else if (errno == EACCES)
					{
						printf("Permission denied\n");
					} else
					{
						printf("Could not open file "
							"to send\n");
					}
					continue;
				}
				// Send opcode
				write(socketID, &tempChar, sizeof(tempChar));
				// Send filename length
				write(socketID, &int2bytes, sizeof(int2bytes));
				// Send filename
				write(socketID, tempToken,
					(sizeof(char) * ntohs(int2bytes)));
				// Get back opcode
				read(socketID, &tempChar, sizeof(tempChar));
				// Get back status code
				read(socketID, &tempChar, sizeof(tempChar));
				if (tempChar == '0') // Can send file
				{
					// Set next opcode and send
					tempChar = 'E';
					write(socketID, &tempChar, 
						sizeof(tempChar));
					// Get file info
					stat(tempToken, &fileInfo);
					int4bytes = fileInfo.st_size;
					// Send file size
					int4bytes = htonl(int4bytes);
					write(socketID, &int4bytes,
						sizeof(int4bytes));
					int4bytes = ntohl(int4bytes);
					bytesRemain = int4bytes;
					// Loop through and send file
					while (bytesRemain)
					{
						if (bytesRemain < NET_BUF_SIZE)
						{
							bytesRead = 
								read(openFile,
								fileBuffer,
								(sizeof(char)
								*
								bytesRemain));
						} else
						{
							bytesRead =
								read(openFile,
								fileBuffer,
								(sizeof(char)
								*
								NET_BUF_SIZE));
						}
						write(socketID, fileBuffer,
							bytesRead);
						bytesRemain = bytesRemain -
							bytesRead;
					}
					// Close the open file
					close(openFile);
				} else if (tempChar == '1') // Filename clash
				{
					printf("A file of that name already "
						"exists on the server\n");
				} else if (tempChar == '2') // Create error
				{
					printf("Server could not create file "
						"for saving\n");
				} else if (tempChar == '3') // Other error
				{
					printf("The server incountered an "
						"unspecified error\n");
				} else // Just in case
				{
					printf("Something went wrong...\n");
				}
			} else if (strcmp("pwd", tempToken) == 0)
			{
				// pwd command stuff here
				// Set opcode in tempChar
				tempChar = 'E';
				// Send opcode
				write(socketID, &tempChar, sizeof(tempChar));
				// Get back opcode response
				read(socketID, &tempChar, sizeof(tempChar));
				// Get back length of pwd
				read(socketID, &int2bytes, sizeof(int2bytes));
				// Get pwd
				read(socketID, tempString, 
					(sizeof(char) * ntohs(int2bytes)));
				// Print out pwd
				printf("%s\n", tempString);
			} else if (strcmp("lcd", tempToken) == 0)
			{
				// lcd command stuff here
				tempToken = strtok(NULL, TOK_DELIM);
				check = chdir(tempToken);
				if (check == -1) // If dir change fails
				{
					if (errno == ENOENT)
					{
						printf("Directory does not "
							"exist\n");
					} else if (errno == EACCES)
					{
						printf("Permission denied\n");
					} else
					{
						printf("Unspecified error. "
							"Directory not "
							"changed\n");
					}
				}
			} else if (strcmp("ldir", tempToken) == 0)
			{
				// ldir stuff here
				numFiles = scandir(".", &fileList, 
					NULL, alphasort);
				if (numFiles == 0)
				{
					perror("myftp:scandir");
				} else
				{
					for (i = 0; i < numFiles; ++i)
					{
						if (fileList[i]->d_type
							== DT_REG)
						{
							printf("f ");
						} else if (fileList[i]->d_type
							== DT_DIR)
						{
							printf("d ");
						} else
						{
							printf("o ");
						}

						printf("%s\n", 
							fileList[i]->d_name);
						free(fileList[i]);
					}

					free(fileList);
				}
			} else if (strcmp("lpwd", tempToken) == 0)
			{
				// lpwd stuff here
				getcwd(tempString, 256);
				printf("%s\n", tempString);
			} else
			{
				// invalid command
				printf("Invalid command\n");
			}
		}
		
	} while (1);
}
