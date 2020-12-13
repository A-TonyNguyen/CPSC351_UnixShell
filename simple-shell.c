/**
 * Simple shell interface program.
 *
 * Operating System sadaspts - Tenth Edition
 * Copyright John Wiley & Sons - 2018
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE		80 /* 80 chars per line, per command */

int main(void)
{
	char *args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
	int hasAmpersand = 0;		// 0 if it does not have an ampersand 1 if there is an ampersand
	int numOfElem = 0;			// Number of Elements for args[]
    int should_run = 1;
	pid_t pid;
		
    	while (should_run)
		{   
        	printf("osh>");
        	fflush(stdout);
        
        	/**
         	 * After reading user input, the steps are:
         	 * (1) fork a child process
         	 * (2) the child process will invoke execvp()
         	 * (3) if command includes &, parent and child will run concurrently
         	 */

			 //Reads user input
			 userInput(args, &hasAmpersand, &numOfElem);

			 pid = fork();

			if(pid == 0)
			{
				//If the array is not empty
				if(numOfElem != 0)
				{
					int file;
					int redirectIO = 0;		//Flag to indicate (1)input or (2)output

					//For checking input '<', output '>', or a pip '|'
					for (int i = 0; i <= numOfElem; i++)
					{
						//***************************************************
						// 3.Adding support for input and output redirection.
						//***************************************************

						//if '<' input
						if(strcmp(args[i], '<') == 0)
						{
							file = open(args[i + 1], O_RDONLY);

							 dup2(file, STDIN_FILENO);
							 args[i] = NULL;
                       		 args[i + 1] = NULL;
                        	 redirectIO = 1;

						}
						//else if '>' output
						else if(strcmp(args[i], '>') == 0)
						{
							file = open(args[i + 1], O_WRONLY | O_CREAT, 0644);

							 dup2(file, STDIN_FILENO);
							 args[i] = NULL;
                       		 args[i + 1] = NULL;
                        	 redirectIO = 2;
						}
						//********************************************************************
						// 4. Allowing the parent and child process to communicate via a pipe
						//********************************************************************
						else if(strcmp(args[i], '|')== 0)
						{
							continue;
						}
					}

					//Close out all files
					if (redirectIO == 1)
                    	close(STDIN_FILENO);

                	else if (redirectIO == 2)
                    	close(STDOUT_FILENO);

                	close(file);
				}
			}
			//If there are 1 or more fork(), wait for child process to exit
			else if(pid > 0)
			{
				//If there is an ampersand then parent and child process runs concurrently
				if(hasAmpersand == 0)
					wait(NULL);
			}
    	}
    
	//Exit the program
	return 0;
}

void userInput(char *args[], int *hasAmpersand, int *numOfElem)
{
	char command[MAX_LINE];		
    int length = 0;
    char delimiter[] = " ";

	length = read(STDIN_FILENO, command, 80);

	//********************************
	// 2. Providing a history feature.
	//********************************
    if (strcmp(command, "!!") == 0)
	{
        if (*numOfElem == 0)
		{
            printf("No commands in history.\n");
        }
        else
		{
			//Print history
		}
    }

	//If reached MAX_LINE limit *do something*


	//Breaks the string (user inputted command) into a series of tokens using the delimiter
	char *ptr = strtok(command, delimiter);

	//Loop
	while(ptr != NULL)
	{
		//If token is an ampersand
		if (ptr[0] == '&')
		{
            *hasAmpersand = 1;
            ptr = strtok(NULL, delimiter);
            //continue;
        }

		*numOfElem += 1;
		args[*numOfElem - 1] = strdup(ptr);
		ptr = strok(NULL, delimiter);			
	}

	args[*numOfElem] = NULL;
}