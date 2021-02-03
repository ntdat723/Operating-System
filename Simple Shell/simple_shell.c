/**********************************************************************************************************************************************************
 * Simple UNIX Shell
 * @author: 
 * 
 **/


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LENGTH 80 // The maximum length of the commands
extern int errno;


int main() {
	char command[MAX_LENGTH];


	char *args[MAX_LENGTH / 2 + 1]; // MAximum 40 argments

	int should_run = 1;

	char *token;				//token from strtok()
	char *sp_arg;				//to save >/</|
	char *remaining;			//remaining token after >/</|
	char *history_buffer;			
	char cpy_cmd[MAX_LENGTH];		//a copy of the command
	bool his = false;			//indicate if there is a command in history
	int sp_flag = 0;			//indicate if there's >/</| in the input
	int amp_flag = 0;				//indicate if there's an ampersand at the end
	
	
	while (should_run) {
		
		//Echo the old command if there is one in the history
		if (his == true)				
		{
			strcpy(command, history_buffer);
			printf("%s", command);
			his = false;
		}
		else
		{
			printf("ssh>>");
			fflush(stdout);
			fgets(command, MAX_LENGTH, stdin);
		}
		strcpy(cpy_cmd, command);
		
		//Check if the Enter key is pressed
		if (strcmp(command, "\n") == 0)
		{
			continue;
		}
		
		//Parse command and arguments.
		int count = 0;
		for (int j = 0; j < strlen(command); ++j)
		{
			//Check if there is an occurence of <, >, or | in the command
			if (command[j] == '<' || command[j] == '>' || command[j] == '|')
			{
				sp_flag = 1;
			}
			
			//Count the number of space in the command
			if (command[j] == ' ')
			{
				++count;
			}
			
			//Check if there is an ampersand in the command
			if (command[j] == '&')
			{
				amp_flag = 1;
				command[j] = '\0';
			}
		}
		
		//Remove the linefeed at the end of each command
		command[strlen(command) - 1] = '\0';
		int i = 1;
		
		//Case if there is a one of >/</| and more than 1 argument
		if (sp_flag == 1 && count != 0)
		{
			args[0] = strtok(command, " ");
			token = args[0];
		
		
			while (token != NULL)
			{
				token = strtok(NULL, " ");
				if (strcmp(token, ">") == 0 || strcmp(token, "<") == 0 || strcmp(token, "|") == 0)
				{
					sp_arg = token;
					break;
				}
				else
				{
					args[i] = token;
					++i;
				}
				
			}
			
			//As args[] must be a NULL-terminated array, args[i] will be assigned with NULL value
			args[i] = NULL;			
			
			//remaining variable is for what is left after >/</|
			remaining = strtok(NULL, "\0");
		
		}
		
		//Case when there is only 1 argument
		else if (count == 0)
		{
			args[0] = strtok(command, "\0");
			sp_arg = NULL;
			remaining = NULL;
			
			//Check whether exit is available or not, if yes, then quit
			if(strcmp(args[0], "exit") == 0)
			{
				should_run = 0;
				continue;
			}
		}
		
		//Case when there are many arguments, but have no >/</|
		else if (count != 0)
		{
			args[0] = strtok(command, " ");
			token = args[0];
			
			while (token != NULL)
			{
				if (count == i)
				{
					token = strtok(NULL, "\0");	
				}
				else
				{
					token = strtok(NULL, " ");
				}
				args[i] = token;
				++i;
			}
			sp_arg = NULL;
			remaining = NULL;
		}
		args[i] = NULL;
		
		
		//Check the existence of !! and if there is any commands in history for history feature
		if (strcmp(args[0], "!!") == 0)
		{
			if (history_buffer == NULL)
			{
				printf("No commands in history!\n");
			}
			else
				his = true;
			continue;
		}
		
				
		
		//If command contains output redirection argument
		//	fork a child process invoking fork() system call and perform the followings in the child process:
		//		open the redirected file in write only mode invoking open() system call
		//		copy the opened file descriptor to standard output file descriptor (STDOUT_FILENO) invoking dup2() system call
		//		close the opened file descriptor invoking close() system call
		//		change the process image with the new process image according to the UNIX command using execvp() system call
		//	If command does not conatain & (ampersand) at the end
		//		invoke wait() system call in parent process.
		//
		//		
		

		if (sp_arg != NULL && remaining != NULL && strcmp(sp_arg, ">") == 0)
		{
			int fd, a;						//fd is for the file descriptor of 											//the read file, a is for execvp 											//returned value
			
			int ifFail = dup(STDOUT_FILENO);			//get the current open file 
										//descriptor for standard output
			pid_t p = fork();				
			switch(p)
			{
			case 0:						//successfully create a child process
				fd = open(remaining, O_CREAT | O_WRONLY, 00700);
				dup2(fd, STDOUT_FILENO);
				close(fd);
				a = execvp(args[0], args);
				if (a == -1)					//if fail to execute the command
				{						//or wrong syntax, restore stdout
					dup2(ifFail, 1);			//to terminal to indicate the error
					close(ifFail);
					printf("An error has occurred.\n");
					exit(EXIT_FAILURE);
				}
				break;
			case -1:						//fail to create
				printf("%s\n", "There's an error.");
				break;
			default:						//returned to parent process
				wait(&p);
				break;
			}
			if (amp_flag == 0)					//if there is an ampersand at the end
			{
				wait(&p);
			}
			
		}

		//If command contains input redirection argument
		//	fork a child process invoking fork() system call and perform the followings in the child process:
		//		open the redirected file in read  only mode invoking open() system call
		//		copy the opened file descriptor to standard input file descriptor (STDIN_FILENO) invoking dup2() system call
		//		close the opened file descriptor invoking close() system call
		//		change the process image with the new process image according to the UNIX command using execvp() system call
		//	If command does not conatain & (ampersand) at the end
		//		invoke wait() system call in parent process.
		//
		//	
		
		
		if (sp_arg != NULL && remaining != NULL && strcmp(sp_arg, "<") == 0)
		{
			int fd, a;						//fd is for the file descriptor of 											//the read file, a is for execvp 											//returned value
			pid_t p = fork();
			switch(p)
			{
			case 0:						//successfully create a child process
				fd = open(remaining, O_RDONLY, 00700);
				if (fd < 0)
				{
					printf("An error has occurred!\n");
					exit(EXIT_FAILURE);
				}
				dup2(fd, STDIN_FILENO);
				close(fd);
				a = execvp(args[0], args);
				if (a == -1)
				{
					printf("Not working\n");
					exit(EXIT_FAILURE);
				}
				break;
			case -1:						//fail to create a child process
				printf("%s\n", "There's an error.");
				break;
			default:						//returned to parent process
				wait(&p);
				break;
			}
			if (amp_flag == 0)					//if there is an ampersand at the end
			{
				wait(&p);
			}
			
		}
		
		//If command contains pipe argument
		//	fork a child process invoking fork() system call and perform the followings in the child process:
		//		create a pipe invoking pipe() system call
		//		fork another child process invoking fork() system call and perform the followings in this child process:
		//			close the write end descriptor of the pipe invoking close() system call
		//			copy the read end  descriptor of the pipe to standard input file descriptor (STDIN_FILENO) invoking dup2() system call
		//			change the process image of the this child with the new image according to the second UNIX command after the pipe symbol (|) using execvp() system call
		//		close the read end descriptor of the pipe invoking close() system call
		//		copy the write end descriptor of the pipe to standard output file descriptor (STDOUT_FILENO) invoking dup2() system call
		//		change the process image with the new process image according to the first UNIX command before the pipe symbol (|) using execvp() system call
		//	If command does not conatain & (ampersand) at the end
		//		invoke wait() system call in parent process.
		//
		//


		if (sp_arg != NULL && remaining != NULL && strcmp(sp_arg, "|") == 0)
		{
			int ppfd[2];
			char* bf_args[1];
			bf_args[0] = strdup(remaining);
			bf_args[1] = NULL;
			pid_t p1 = fork();			

			//NULL-terminated array for command after pipe symbol |
			switch(p1)
			{
			case 0:
				pipe(ppfd);
				pid_t p2 = fork();
				switch(p2)
				{
					case 0:				//successfully create a child process
						close(ppfd[1]);
						dup2(ppfd[0],  STDIN_FILENO);
						execvp(bf_args[0], bf_args);
						break;
					case -1:				//fail to create a child process
						printf("%s\n", "There's an error.");
						break;
					default:				
						wait(&p2);
						break;
				}
				close(ppfd[0]);
				dup2(ppfd[1], STDOUT_FILENO);
				if (execvp(args[0], args) < 0)
				{
					printf("An error has occurred.\n");
					exit(EXIT_FAILURE);
				}
				break;
			case -1:						//fail to create a child process
				printf("%s\n", "There's an error.");
				break;
			default:						//returned to the parent process
				wait(&p1);
				break;
			}
			
			if (amp_flag == 0)					//if there is an ampersand at the end
			{
				wait(&p1);
			}
			
		}




		//If command does not contain any of the above
		//	fork a child process using fork() system call and perform the followings in the child process.
		//		change the process image with the new process image according to the UNIX command using execvp() system call
		//	If command does not conatain & (ampersand) at the end
		//		invoke wait() system call in parent process.
		
		
		history_buffer = strdup(cpy_cmd);				//Get the previous command into 											//buffer
		if (sp_arg == NULL && remaining == NULL)
		{
			int status;
			pid_t p = fork();
			switch(p)
			{
				case 0:					//successfully create a child process
					status = execvp(args[0], args);	//get if command is successfully 											//executed
					if (status < 0)
					{
						printf("Invalid command.\n");
						exit(EXIT_FAILURE);
					}
					break;
				case -1: 					//fail to create a child process
					printf("There's an error.\n");
					break;
				default: 					//returned to parent process
					wait(&p);
					break;
			}
			if (amp_flag == 0)					//check if there is an ampersand
			{
				wait(&p);
			}
		}	
	}

	return 0;
}
