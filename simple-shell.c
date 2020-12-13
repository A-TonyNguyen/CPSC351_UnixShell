/**
 * Program: Simple shell interface program.
 * Program Description: This project consists of designing a C program to serve as a shell interface 
 * that accepts user commands and then executes each command in a separate process. Your implementation 
 * will support input and output redirection, as well as pipes as a form of IPC between a pair of commands.
 * Completing this project will involve using the UNIX fork(), exec(), wait(), dup2(), and pipe() system 
 * calls and can be completed on any Linux, UNIX, or macOS system.
 * 
 * Author: Tony Nguyen
 * Email: Tonyxd14@csu.fullerton.edu
 * Date: October 1, 2020
 *
 * Operating System - Tenth Edition
 * Copyright John Wiley & Sons - 2018
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_LINE	80 /* 80 chars per line, per command */
#define READ_END	0
#define WRITE_END	1

//Function that calls for user input to fill array
void userInput(char *args[], bool *hasAmpersand, int *numOfElem)
{
	char command[MAX_LINE];		//char command array with max of 80 char
    int length = read(STDIN_FILENO, command, 80);
    char delimiter[] = " ";

	// Removes trailing newline character
	if (length > 0 && command[length - 1] == '\n')
    {
        command[length - 1] = '\0';
    }

	// ********************************
	// 2. Providing a history feature.
	// ********************************
    if (strcmp(command, "!!") == 0)
	{	
		//Error handling
        if (*numOfElem == 0)
		{
            printf("No commands in history.\n");
        }
		return;
    }

	for(int i = 0; i < *numOfElem; i++)
	{	
		args[i] = NULL;
	}
	
    *numOfElem = 0;
    *hasAmpersand = false;

	//pointer
	char *ptr = strtok(command, delimiter);

	//Breaks the string (user inputted command) into a series of tokens using the delimiter
	while(ptr != NULL)
	{
		//If token is an ampersand
		if (ptr[0] == '&')
		{
            *hasAmpersand = true;				//Flag if program detects an ampersand
            ptr = strtok(NULL, delimiter);		//If no pointer is returned, set ptr to NULL
            //continue;
        }

		args[*numOfElem] = strdup(ptr);			//Sets elements inside array
		*numOfElem += 1;						//Increase number of elements
		ptr = strtok(NULL, delimiter);			//If no pointer is returned, set ptr to NULL
	}

	args[*numOfElem] = NULL;
}

// Main Program
int main(void)
{
	char *args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
	bool hasAmpersand = false;		// 0 if it does not have an ampersand 1 if there is an ampersand
	int numOfElem = 0;			// Number of Elements for args[]
	bool usePipe = false;		//Flag to indicate we are using a pipE
    int should_run = 1;
	pid_t pid;
		
    while (should_run)
	{   
		usePipe = false;
        printf("osh>");
        fflush(stdout);
        
        /**
         * After reading user input, the steps are:
         * (1) fork a child process
         * (2) the child process will invoke execvp()
         * (3) if command includes &, parent and child will run concurrently
         */

		 //Creates tokens that are put into an array fro the user input
		 userInput(args, &hasAmpersand, &numOfElem);

		// ***************************************************
		// 1. Forks a child process
		// ***************************************************
		 pid = fork();

		//If pid < 0 then an error has occured
		if(pid < 0)
		{
			fprintf(stderr, "Fork Failed"); 
			return 1;
		}
		//If pid == 0 then it is a child process
		else if(pid == 0)
		{
			//If the array is not empty
			if(numOfElem != 0)
			{
				int file;						//File name
				bool redirectInput = false;		//Flag to indicate intput file
				bool redirectOutput = false; 	//Flag to indicate output outfile

				//Loops the array to check for input '<', output '>', or a pip '|'
				for (int i = 0; i < numOfElem; i++)
				{
				// ***************************************************
				// 3.Adding support for input and output redirection.
				// ***************************************************

					//if '<' input
					if(strcmp(args[i], "<") == 0)
					{
						file = open(args[i + 1], O_RDONLY);				//Will only read the file

						if (args[i + 1]  == NULL || file == -1)
						{
                           	printf("Invalid Command!\n");
                           	return 1;
                       	}

						dup2(file, STDIN_FILENO);	//Any writes in standard output will be sent to "out.txt"
						redirectInput = true;	
						args[i] = NULL;				//Sets "<" to NULL
                 		args[i + 1] = NULL;			//Sets file descriptor to NULL

						break;
					}
					//else if '>' output
					else if(strcmp(args[i], ">") == 0)
					{
						file = open(args[i + 1], O_WRONLY | O_CREAT);	//Will create and write to file

						//Error handling: Checks if there is a filename after "<"
						if (args[i + 1] == NULL || file == -1)
						{
                           	printf("Invalid Command!\n");
                      	 	return 1;
                    	}

						dup2(file, STDOUT_FILENO);	//Any writes in standard output will be sent to "out.txt"
						redirectOutput = true;
						args[i] = NULL;				//Sets "<" to NULL
                   		args[i + 1] = NULL;			//Sets file descriptor to NULL

						break;
					}
					// ********************************************************************
					// 4. Allowing the parent and child process to communicate via a pipe
					// ********************************************************************
					else if(strcmp(args[i], "|") == 0)
					{
						usePipe = true;
                    	int fd[2];

						//Error Handling for pipe
                   		if (pipe(fd) == -1)
						{
							fprintf(stderr,"Pipe failed");
							return 1;
						}
						
						//Create two arrays for the first and second set of commands
                    	char *args1[i + 1];
                    	char *args2[numOfElem - i + 1];

						//Fill first array
                    	for (int j = 0; j < i; j++)
						{
                        	args1[j] = args[j];
                    	}					
                    	args1[i] = NULL;

						//Fill second array
                    	for (int j = 0; j < numOfElem - i - 1; j++)
						{
                       	 args2[j] = args[j + i + 1];
                    	}
                    	args2[numOfElem - i - 1] = NULL;

						//Now fork a child process
                    	pid_t pidPipe = fork();
						
						//Error handling
						if (pidPipe < 0)
						{
							fprintf(stderr, "Fork failed.");
							return 1;
						}
                    	else if (pidPipe > 0)
						{
							//Wait for child process to complete
                       		wait(NULL);
							//Close unused pipe
                       		close(fd[WRITE_END]);
                        	dup2(fd[READ_END], STDIN_FILENO);
                        	close(fd[READ_END]);

							//Error Handling: Invalid input for the second command
                        	if (execvp(args2[0], args2) == -1)
							{
                            	printf("Invalid Command!\n");
                            	return 1;
                        	}
                    	}
						else
						{
							//Close unused pipe
                        	close(fd[READ_END]);
                      		dup2(fd[WRITE_END], STDOUT_FILENO);
                        	close(fd[WRITE_END]);

							//Error Handling: Invalid input for the first command
                        	if (execvp(args1[0], args1) == -1)
							{
                           		printf("Invalid Command!\n");
                            	return 1;
                        	}
                       	}
                    	close(fd[READ_END]);
                    	close(fd[WRITE_END]);
						
                    	break;
					}
				}

				//If there are no redirects
                if (usePipe == false)
				{
					//Error handling: Check if code in salid
                    if (execvp(args[0], args) == -1)
					{
                        printf("Invalid Command!\n");
                        return 1;
                    }
                }

				//Close out input and outputfiles
                if (redirectInput == false) 
				{
                    close(STDIN_FILENO);

                }
				else if (redirectOutput == false)
				{
                    close(STDOUT_FILENO);
                }
                close(file);
			}
		}
		//Parent Process: 
		else
		{
			//If there is no ampersand '&', then parent will wait for the child to complete
			if(hasAmpersand == false)
			{
				wait(NULL);
			}
		}
    }

	//Exit the Program
	return 0;
}