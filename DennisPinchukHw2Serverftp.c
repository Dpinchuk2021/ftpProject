/* 
 * server FTP program
 *
 * NOTE: Starting homework #2, add more comments here describing the overall function
 * performed by server ftp program
 * This includes, the list of ftp commands processed by server ftp.
 * 
 * 
 * Name: Dennis Pinchuk
 * Class: CSCI 477 Framingham State University
 * Professor: Krishna Suban
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define SERVER_FTP_PORT 5017 /*When clients want to connect to the server, they will use this port number to establish a control connection.*/
#define DATA_CONNECTION_PORT 5018 /*During an FTP session, when clients want to transfer data (like uploading or downloading files), they will use this port number to establish a separate data connection.*/


/* Error and success codes */
#define OK 0
#define ER_INVALID_HOST_NAME -1
#define ER_CREATE_SOCKET_FAILED -2
#define ER_BIND_FAILED -3
#define ER_CONNECT_FAILED -4
#define ER_SEND_FAILED -5
#define ER_RECEIVE_FAILED -6
#define ER_ACCEPT_FAILED -7


/* Function prototypes */

int svcInitServer(int *s);
int clntConnect(char *serverName, int *s);
int sendMessage (int s, char *msg, int  msgSize);
int receiveMessage(int s, char *buffer, int  bufferSize, int *msgSize);


/* List of all global variables */

char userCmd[1024];	    /* User entered FTP command received from the client */
char userCmdCopy[1024]; /* Utilized for temporary string manipulation */
char chgCmd[1024];      /* Utilized for converting a command into a UNIX-compatible command */
char *cmd;   		    /* FTP command extracted from userCmd, without an argument. */
char *argument;         /* Argument extracted from userCmd, without the FTP command. */
char replyMsg[1024];    /* Buffer for sending response messages to the client */ /*4096*/
char ftpData[4096];		/* Buffer for sending and receiving file data to and from the client which is set to 4 bytes */ /*4096*/
int  ftpBytes      = 0; /* Utilized for tallying the total bytes transferred during FTP */
int  fileBytesRead = 0; /* The number of bytes read using fread */
int  bytesReceived = 0; /* The count of bytes received in a single FTP message. */
char testpass[1024];     /* A second password array storing memory that will be allocated to the string*/ /*4096*/

FILE *filePtr;                                       /* A pointer to a temporary file that stores the output of logged commands */
int  bytesRead    = -1;                              /* The number of bytes read by the fread() function */
char fileData[1024];                                 /* A buffer for storing the byte data retrieved by fread(), with a maximum size of 4096 */ /*4096*/

/* Login Codes */
#define LOGGED_IN 1   /*Represents a value (1) for the status when a user is logged in */
#define LOGGED_OUT 0  /*Represents a value (0) for the status when a user is logged out */
#define NO_USER -1    /*Represents a value (-1) for the status when there is no user associated with a particular action or session */

/*
 * main
 *
 * Function to listen for connection request from client
 * Receive ftp command one at a time from client
 * Process received command
 * Send a reply message to the client after processing the command with staus of
 * performing (completing) the command
 * On receiving QUIT ftp command, send reply to client and then close all sockets
 *
 * Parameters
 * argc		- Count of number of arguments passed to main (input)
 * argv  	- Array of pointer to input parameters to main (input)
 *		   It is not required to pass any parameter to main
 *		   Can use it if needed.
 *
 * Return status
 *	0			- Successful execution until QUIT command from client 
 *	ER_ACCEPT_FAILED	- Accepting client connection request failed
 *	N			- Failed stauts, value of N depends on the command processed
 */

int main(int argc, char *argv[] )
{

	int msgSize;        /* Size of msg received in octets (bytes) */
	int listenSocket;   /* listening server ftp socket for client connect request */
	int ccSocket;        /* Control connection socket - to be used in all client communication */
	int dcSocket;       /* Data connection socket - to be used in all server communication */
	int status;
    ccSocket = -1;    /* ensure that cc socket are initially set to an invalid value. */
    listenSocket = -1; /* ensure that listen socket are initially set to an invalid value. */

	char *userList[4]  = {"Dennis", "Eric", "Steven", "Mike"}; /* The list of valid user names set to an array called userList */
	char *passList[4]  = {"JackSparrow", "isSunny46", "graygoose", "bluelabel45"};            /* The list of passwords associated with the index of userList set to another array called passList */
	int  isLoggedIn   = -1;                              /* -1 (default), 0 if the User is not logged in, 1 otherwise. */
	int  userIndex    = -1;                              /* The index of userList that identifies the current user. See Login Codes. */



	/*
	 * NOTE: without \n at the end of format string in printf,
         * UNIX will buffer (not flush)
	 * output to display and you will not see it on monitor.
	*/
	printf("Started execution of server ftp\n"); /*Prints out that the server has started when executing the executable*/


	 /*initialize server ftp*/
	printf("Initialize ftp server\n");	/* Print a message indicating that the server FTP program has started execution */

	status=svcInitServer(&listenSocket); /*Call the svcInitServer function to initialize the server and store the result in the status variable*/
	if(status != 0)
	{
		printf("Exiting server ftp due to svcInitServer returned error\n"); /*Print a message indicating that the server FTP program is exiting due to an error returned by the svcInitServer function */
		exit(status); /*exiting the loop*/
	}


	printf("ftp server is waiting to accept connection\n"); /*Print a message indicating that the ftp server is connected and wating to accept a connection*/

	/* wait until connection request comes from client ftp */
	ccSocket = accept(listenSocket, NULL, NULL);

   /* printf("Accepted connection, ccSocket: %d\n", ccSocket); Debugging for ls*/

	printf("Came out of accept() function \n"); /*Print a message indicating that the program has successfully returned from the accept() function*/

	if(ccSocket < 0) /*Check if the control connection socket is less than 0, indicating an error during connection acceptance*/
	{
		perror("cannot accept connection:"); /*Display an error message about the failed connection acceptance*/
		printf("Server ftp is terminating after closing listen socket.\n"); /*Notify that the server is terminating after closing the listen socket*/
		close(listenSocket);  /* close the listen socket */
		return (ER_ACCEPT_FAILED);  /* return an error exit */ 
	}

	printf("Connected to client, calling receiveMsg to get ftp cmd from client \n"); /*Print a message indicating a successful connection to the client and that the server is now waiting for an FTP command from the client*/


	/* Receive and process ftp commands from client until quit command.
 	 * On receiving quit command, send reply to client and 
         * then close the control connection socket "ccSocket". 
	 */
	do
	{
        /*Call receiveMessage function to get the FTP command from the client, and store the command, status, and message size in respective variables*/
 	    status=receiveMessage(ccSocket, userCmd, sizeof(userCmd), &msgSize); 
	    if(status < 0)
	    {
		    printf("Receive message failed. Closing control connection \n");
		    printf("Server ftp is terminating.\n"); 
		    break; /*Break out of the if statement so there is no infinite loop*/
	    }

        /*
        * Starting Homework#2 program to process all ftp commands must be added here.
        * See Homework#2 for a list of FTP commands to implement.
        */

	    printf("Message received. status: %d.\n", status); /* The client is informed that their message has been received by the program. */

	    if (userCmd == NULL) {
    			printf("userCmd is NULL.\n"); /* There is no valid command or argument will be printed */
	    } else if (strcmp(userCmd, "") == 0) {
    			printf("userCmd is the empty set.\n"); /* otherwise, the program will print out an empty set */
	    }

	    /* "Extract the command and argument from the 'userCmd' variable using strtok */
        strcpy(userCmdCopy, userCmd); /* Copies the content of the userCmd string into userCmdCopy.*/
		cmd = strtok(userCmdCopy, " "); /*Tokenizes userCmdCopy using a space as the delimiter, and stores the first token (command) in the cmd variable.*/
		/* argument = strtok(NULL, " "); */
        
        strcpy(userCmdCopy, userCmd); /*It seems redundant but my code would give a code dump error without this line*/
	    cmd = strtok(userCmdCopy, " "); /*Same with this line as well*/
        /*If no valid command was provided.*/
	    if (cmd == NULL) {
    		printf("cmd is NULL after strtok.\n"); /*Prints a message to the console indicating that cmd is NULL after tokenization.*/
    		continue; /*Skips the rest of the loop iteration and starts the next iteration, since no valid command was found.*/
	    }

	    argument = strtok(NULL, " "); /**/
        /*This is where all 13 commands are implemented via hard code for testing purposes.*/ 
	    if (argument == NULL && (strcmp(cmd, "pass") == 0 || strcmp(cmd, "user") == 0 || strcmp(cmd, "stat") == 0 || strcmp(cmd, "mkdir") == 0 || strcmp(cmd, "rmdir") == 0 || strcmp(cmd, "dele") == 0 || strcmp(cmd, "cd") == 0 || strcmp(cmd, "pwd") == 0 || strcmp(cmd, "help") == 0)) {
            /* Handle the case where the argument is required for these specific commands */
            printf("Missing argument for command: %s.\n", cmd);
            /* continue; */
        }

	    /* Output a message indicating that the server has received the command */

	    printf("Received command: %s.\n", userCmd); /*prints the received command stored in the variable userCmd as a string*/
	    printf("cmd: %s. \n", cmd); /*prints the parsed command stored in the variable cmd as a string*/
	    printf("arg: %s.\n", argument); /*prints the parsed argument(s) stored in the variable argument as a string*/

	    /* The initial conditional statement to assist with handling the command in the server. The beginning of 1 out of 13 of these commands. */
	    /* Establish the user command if the user is authenticated/authorized*/   

        /*If the received command is "help", send a list of supported commands in the replyMsg variable. */
	    if(strcmp(cmd, "help") == 0) {
		    strcpy(replyMsg, "214 user stat mkdir rmdir dele cd help ls pwd pass \n");
		}

        /* Handle "user" command: process the username provided by the client. */
		else if(strcmp(cmd, "user") == 0) {
    			isLoggedIn = NO_USER;
    			userIndex = -1;
                /* Check if an argument (username) is provided. */
    			if(argument[0] == NULL || strcmp(argument, "") == 0) {
        			strcpy(replyMsg, "501 No username given. Usage: \"user <username>\".\n");
        			userIndex = -1;
    			}else {
        			int i = 0;
				    /* int numUsers = 4; */
        			int numUsers = sizeof(userList) / sizeof(*userList); /* Calculate the number of users in the userList array. */
                    char *local_argument = strdup(argument); /*use the strdup() function to create a copy of argument and then use that copy in the loop.*/

                    /* Loop through the userList to find a match for the provided username. */
        			for(; i < numUsers; i++) {
                        /*The following three lines where used in debugging*/
                        /*printf("userList[%d]: %s, address: %p\n", i, userList[i], (void *)userList[i]);*/
                        /*printf("local_argument: %s, address: %p\n", local_argument, (void *)local_argument);*/
                        /*printf("Length of userList[i]: %lu, Length of local_argument: %lu\n", strlen(userList[i]), strlen(local_argument));*/

                        /* If a match is found, store the corresponding password and update the userIndex and isLoggedIn variables. */
            			if(strncmp(userList[i], local_argument, strlen(userList[i])) == 0) {
                            strcpy(testpass, passList[i]);
                			userIndex = i;
                			isLoggedIn = LOGGED_OUT;
                			strcpy(replyMsg, "331 User name good, need password.\n");
                			break;
            			}
        		    }
                    free(local_argument); /*Free up the allocation of memory*/

                    /* If the userIndex is still -1, no matching username was found. */
        		    if(userIndex < 0) {
            	        strcpy(replyMsg, "530 Invalid username. Not logged in. \n");
        		    }
    		    }
	        }


	    /* The 'password' command is processed as 'pass' and can only be executed after a user successfully enters their username.
	    * If a user logs into the server but provides an incorrect password with the 'pass' command, they will be logged out of the server.
	    * However, if the user provides a valid password, the server will flag them as logged in.
	    */
	    
	    /* If no password is provided, the user will be notified of a syntax error */

        /* Handle "pass" command: process the password provided by the client. */
	    else if(strcmp(cmd, "pass") == 0){
		printf("Processing pass command - empty argument.\n"); /*If the user does not provide a password as pass, then display the following replymsg that there is no argument*/

		/* Check if a password argument is provided.NOTE: The program will inform the user of a syntax error if they fail to provide a password or an "empty" password. */
		if(argument == NULL || argument[0] == '\0' || strcmp(argument, "") == 0) {
			strcpy(replyMsg, "501 Syntax error. Use: \"pass <password>\". \n");
		}
		/* If no valid "user" command was sent previously, inform the user of a bad command sequence. */
		else if(isLoggedIn == NO_USER || userIndex < 0){
			printf("Processing pass command - no user or invalid userIndex.\n");
			isLoggedIn = LOGGED_IN; 
			strcpy(replyMsg, "503 Bad sequence of commands. Use \"user <username>\" first.\n");
		}
		/* If the password matches the one for the selected user, set the isLoggedIn status to LOGGED_IN and send a success message. */
		/* else if(strcmp(passList[userIndex], argument) == 0) { */
        else if(strcmp(testpass, argument) == 0) {
			printf("Processing pass command - successful login.\n");
			isLoggedIn = LOGGED_IN; /* indicating that a user has successfully logged into the server. */ 
			strcpy(replyMsg, "230 User logged in, proceed.\n");
		}
        
		/* Handle invalid password or unexpected error cases. If the user provides an invalid command, error messages are displayed, but the 'userIndex' variable remains unchanged, and the 'isLoggedIn' variable is set to 'LOGGED_OUT' to ensure that the user is logged out of the server. */
		else{
			printf("Processing pass command - invalid command.\n");
			isLoggedIn = LOGGED_OUT;
            userIndex = -1;
            /* If userIndex is still valid, inform the user of an invalid password and prompt to retry. */
			if(userIndex >= 0) {
				strcpy(replyMsg, "530 Invalid password for ");
				strcat(replyMsg, userList[userIndex]);
				strcat(replyMsg, ". Use \"pass\" command to try it again, or \"user\" command to switch to new users.\n");
			}
			/* In case of an unexpected error, inform the user and set the command to "quit" for system shutdown. */
			else {
				strcpy(replyMsg, "421 An error occurred. Please contact the system admin.");
				strcpy(cmd, "quit");
				}
			}
	    }

        /*If the user is logged in then the following if else block will be able to execute*/
        else if(isLoggedIn == LOGGED_IN){
            

        
	        /* Implmenting the stat command in the system */
	        /* Indicates that the transfer mode for the FTP connection is set to ASCII mode */
            if(strcmp(cmd, "stat") == 0 || strcmp(cmd, "status") == 0) {
                strcpy(replyMsg, "211 Transfer mode is ASCII. \n");
            }

            /* Implementing the mkdir command in the system */
            /* checks if the user command is either "mkdir" or "Mkd". If the user command matches either of these strings, then the code checks if the 'argument' variable is empty or not. */
            else if (strcmp(cmd, "mkdir") == 0 || strcmp(cmd, "Mkd") == 0) {
                if (argument[0] == NULL || strcmp(argument, "") == 0) {
                    /* If the argument is empty or NULL, inform the user of a syntax error. */
                    strcpy(replyMsg, "501 Syntax error. Use: \"mkdir <directory_path>\".\n");
                }
                /* If the user is logged in, proceed to create the directory. */
                else if (isLoggedIn == LOGGED_IN) {
                    /* If the command is "mkd", modify the command to "mkdir" before executing. */
                    if (strcmp(cmd, "mkd") == 0) {
                    strcpy(chgCmd, "mkdir "); /*Helps convert mkd to mkdir using strcpy*/
                    strcat(chgCmd, argument); /*Helps to convert the argument alongside mkd */
                    status = system(chgCmd);
                } else {
                    /* Execute the "mkdir" command as is. */
                    status = system(userCmd);
                }
                if (status == 0) {
                    /* If the status is 0 (successful), inform the user that the directory was created. */
                    strcpy(replyMsg, "257 Created the directory \"");
                    strcat(replyMsg, argument);
                    strcat(replyMsg, "\".\n");
                } else {
                    /* If the status is not 0 (failed), inform the user that the directory couldn't be created. */
                    strcpy(replyMsg, "500 Unable to create directory \"");
                    strcat(replyMsg, argument);
                    strcat(replyMsg, "\".\n");
                    }
                } 
            else {
                    /* If the user is not logged in, inform them that they need to log in first. */
                    strcpy(replyMsg, "530 Not logged in.\n");
                }
        }

            /* Implementing the rmdir command in the system */
            /* Check if the user command is "rmdir" or "rmd". */
            else if (strcmp(cmd, "rmdir") == 0 || strcmp(cmd, "rmd") == 0) {
                /* If the argument is empty or NULL, inform the user of a syntax error. */
                if (argument[0] == NULL || strcmp(argument, "") == 0) {
                    strcpy(replyMsg, "501 Syntax error. Use: \"rmdir <directory_path>\".\n");
                }
                /* If the user is logged in, proceed to remove the directory. */
                else if (isLoggedIn == LOGGED_IN) {
                    /* If the command is "rmd", modify the command to "rmdir" before executing. */
                    if (strcmp(cmd, "rmd") == 0) {
                        strcpy(chgCmd, "rmdir "); /*Helps convert rmd to rmdir using strcpy*/
                        strcat(chgCmd, argument); /*Helps to convert the argument alongside rmd */
                        status = system(chgCmd);
                    } else {
                        /* Execute the "rmdir" command as is. */
                        status = system(userCmd);
                    }

                    /* If the status is 0 (successful), inform the user that the directory was removed. */
                    if (status == 0) {
                        strcpy(replyMsg, "250 Removed the directory \"");
                        strcat(replyMsg, argument);
                        strcat(replyMsg, "\".\n");
                    } else {
                        /* If the status is not 0 (failed), inform the user that the directory couldn't be removed. */
                        strcpy(replyMsg, "500 Unable to remove \"");
                        strcat(replyMsg, argument);
                        strcat(replyMsg, "\".\n");
                    }
                } else {
                    /* If the user is not logged in, inform them that they need to log in first. */
                    strcpy(replyMsg, "530 Not logged in.\n");
                    }
            }

            /* Implementing the cd command in the system */
            else if(strcmp(cmd, "cd") == 0){
                /* Check if the user is logged in. */
                if(isLoggedIn == LOGGED_IN){
                    /* If the argument is NULL, empty, or not provided, change the directory to the user's HOME directory. */
                    if(argument == NULL || argument[0] == NULL || strcmp(argument, "") == 0){
                        /* If the argument is provided, attempt to change to the specified directory. */
                        status = chdir(getenv("HOME")); /*if no argument is provided, process that command using get*/
                        char home_dir[] = "HOME directory"; /*since no argument was provided set it to home directory*/
                        argument = home_dir;
                       /* strcpy(argument, "HOME directory");*/
                    }else{
                        /* If the argument is provided, attempt to change to the specified directory. */
                        status = chdir(argument);
                    }
                    /* If the change directory command is successful, inform the user that the working directory has been changed. */
                    if(status == 0){
                        strcpy(replyMsg, "250 Current working directory changed to \"");
                        strcat(replyMsg, argument);
                        strcat(replyMsg, "\".\n");
                    }
                    else{
                        /* If the change directory command fails, inform the user that the specified directory couldn't be changed to. */
                        strcpy(replyMsg, "500 Unable to change to the specified directory.\n");
                    } 
                    /* If the directory does not exist then pass along that message. */
                }
                else{
                    /* If the user is not logged in, inform them that they need to log in first. */
                    strcpy(replyMsg, "530 Not logged in.\n");
                }
            }

            /* Implementing the pwd command in the system */
            else if(strcmp(cmd, "pwd") == 0){
                /* printf("Entered 'pwd' command block.\n"); */
                /* Check if the user is logged in. */
                if(isLoggedIn == LOGGED_IN){
                    /* If the argument is not NULL, inform the user that the 'pwd' command does not require an argument. */
                    if (argument != NULL) {
                        strcpy(replyMsg, "501 Syntax error: 'pwd' command does not require an argument.\n");
                    } else{
                        /* Execute the 'pwd' command and save the output to a temporary file. */
                        status = system("pwd > ./temp");
                    }
                    /* If the 'pwd' command executed successfully, read the temporary file. */
                    if(status == 0){
                        filePtr = fopen("temp", "r");
                        /* If the temporary file cannot be opened, inform the user of the failure. */
                        if(filePtr == NULL){
                            strcpy(replyMsg, "500 Unable to display current working directory (filePtr missing).\n");
                        }
                        /* If the file doesn't exist or couldn't be opened, notify the user of the failure. */
                        else{
                            /* Read the temporary file and store the data in 'fileData'. */
                            bytesRead = fread(fileData, 1, 4095, filePtr); /* Leave one byte for null-terminator */
                            /* If the file is empty or couldn't be read, inform the user of the failure. */
                            if(bytesRead <= 0){
                                strcpy(replyMsg, "500 Unable to display current directory (read error).\n");
                            }
                            /* Read output file. */
                            else{
                                /* If the file is read successfully, send the user the server response with the current working directory. */
                                fileData[bytesRead] = '\0'; /* NULL-terminate the file data */
                                strcpy(replyMsg, "257 ");
                                strcat(replyMsg, fileData);
                                strcat(replyMsg, "\n");
                            } 
                            /* File read successfully. Send the user the server response. */
                        }
                        /* Clean up the file data and remove it */
                        fclose(filePtr);
                        system("rm ./temp");
                    }
                    else{
                        strcpy(replyMsg, "500 Unable to display current directory (can not process command).\n");
                    }
                    /* The output file could not be created. Notify the user that it failed */
                }
                else{
                    /* If the user is not logged in, inform them that they need to log in first. */
                    strcpy(replyMsg, "530 Not logged in.\n");
                }
            }
            
            /* Implementing the ls command in the system */
            else if(strcmp(cmd, "ls") == 0){
                /* Check if the user is logged in. */
                printf("Entered 'ls' command block.\n");
                if(isLoggedIn == LOGGED_IN){
                    /* If the argument is "-l", execute the 'ls -l' command and save the output to a temporary file. */
                    if (argument != NULL && strcmp(argument, "-l") == 0) {
                        status = system("ls -l > ./temp"); /* Modify this line to include the "-l" flag */
                    } else{
                        /* Execute the 'ls' command and save the output to a temporary file. */
                        status = system("ls > ./temp");
                    }
                    /* If the 'ls' or 'ls -l' command executed successfully, read the temporary file. */
                    if(status == 0){
                        filePtr = fopen("temp", "r");
                        /* If the temporary file cannot be opened, notify the user that the current directory is empty. */
                        if(filePtr == NULL){
                            strcpy(replyMsg, "The current directory is empty.");
                        }
                        /* If the file doesn't exist or couldn't be opened, notify the user of the failure. */
                        else{
                            /* Read the temporary file and store the data in 'fileData'. */
                            bytesRead = fread(fileData, 1, 4095, filePtr); /* Leave one byte for null-terminator */
                            /* If the file is empty or couldn't be read, inform the user of the failure. */
                            if(bytesRead <= 0){
                                strcpy(replyMsg, "500 Unable to display current directory (read error).\n");
                            }
                            /* Read output file. */
                            else{
                                /* If the file is read successfully, send the user the server response with the directory contents. */
                                fileData[bytesRead] = '\0'; /* NULL-terminate the file data */
                                strcpy(replyMsg, "257 ");
                                strcat(replyMsg, fileData);
                                strcat(replyMsg, "\n");
                            } 
                            /* File read successfully. Send the user the server response. */
                        }
                        /* Clean up the file data and remove the temp file */
                        fclose(filePtr);
                        system("rm ./temp");
                    }
                    else{
                        /* If the 'ls' or 'ls -l' command could not be executed, inform the user of the failure. */
                        strcpy(replyMsg, "451 Unable to display directory contents (can not process command).\n");
                    }
                }
                else{
                    /* If the user is not logged in, inform them that they need to log in first. */
                    strcpy(replyMsg, "530 Not logged in.\n");
                }
            }
            
            /* Implementing the dele command in the system */
            else if(strcmp(cmd, "dele") == 0){
                /* Check if the argument is empty or not. */
                if(argument[0] == NULL || strcmp(argument, "") == 0){
                        strcpy(replyMsg,"501 Syntax error. Use: \"dele <file_path>\".\n");
                }
                /* Check if the user is logged in. */
                else if(isLoggedIn == LOGGED_IN){
                    /* Construct the 'rm' command to remove the specified file. */
                    strcpy(chgCmd, "rm ");
                    strcat(chgCmd, argument);
                    /* Execute the 'rm' command and check its status. */
                    status = system(chgCmd);
                    /* If the 'rm' command executed successfully, inform the user that the file has been removed. */
                    if(status == 0){
                        strcpy(replyMsg, "250 Removed the file \"");
                        strcat(replyMsg, argument);
                        strcat(replyMsg, "\".\n");
                    }
                    /* If the 'rm' command could not be executed, inform the user that the file could not be removed. */
                    else{
                        strcpy(replyMsg, "500 Unable to remove \"");
                        strcat(replyMsg, argument);
                        strcat(replyMsg, "\".\n");
                    }
                }
                else{
                    /* If the user is not logged in, inform them that they need to log in first. */
                    strcpy(replyMsg, "530 Not logged in.\n");
                }
            }
            
            /* Implementing the send command in the system */
            else if(strcmp(cmd, "send") == 0){
            
                /* Attempt a data connection even in the presence of errors to avoid the client from continuously listening. */
                printf("Calling clntConnect to connect to the client.\n");
                status=clntConnect("10.3.200.17", &dcSocket);  /* Off-campus IP: "134.241.37.12" */
                if(status != 0){
                    /* Notify the client that the data connection has failed, close the data connection socket. */
                    strcpy(replyMsg, "425 'send' could not open data connection. Closing data connection socket.\n");
                }
                else{
                    /* Ensure that the user is logged in and has provided a filename after establishing the data connection. */
                    printf("Data connection established to client.\n");
                    if(argument[0] == NULL || strcmp(argument, "") == 0){
                        /* Discard any message received on dcSocket due to an invalid argument. */
                        strcpy(replyMsg, "501 Invalid syntax. Use: \"send <filename>\". Closing data connection.\n");
                    }
                    else if(isLoggedIn == LOGGED_IN){
                        /* Check if the specified file can be written to after confirming that the user is logged in. */
                        filePtr = NULL;
                        filePtr = fopen(argument, "w");
                        if(filePtr == NULL){
                            /* If it's impossible to write to the specified file, discard any message received on the data connection socket.*/
                            strcpy(replyMsg, "550 The file \"");
                            strcat(replyMsg, argument);
                            strcat(replyMsg, "\" could not be opened/created. Closing data connection.\n");
                        }else{
                            /* Start receiving the file data when the file is confirmed to be writable, and continue until no more data is received or no message can be retrieved.*/
                            ftpBytes = 0; /* Set the byte count to its initial value. */
                            printf("Waiting for file transmission from client.\n");
                            do{
                                bytesReceived = 0;
                                status = receiveMessage(dcSocket, ftpData, sizeof(ftpData), &bytesReceived);
                                fwrite(ftpData, 1, bytesReceived, filePtr);
                                ftpBytes = ftpBytes + bytesReceived;
                            }while(bytesReceived > 0 && status == OK); /* Terminate the loop for reading data from the data connection. */
                            sprintf(replyMsg, "226 Received %d", ftpBytes);
                            strcat(replyMsg, " bytes. Closing data connection.\n");
                        }
                        fclose(filePtr); /* Close the file, regardless of whether it was successfully received or not. */
                    }
                    else{
                        /* If the user is not logged in, discard any message received on the dcSocket. */
                        strcpy(replyMsg, "530 Not logged in. Closing data connection.\n");
                    }
                }
                printf("Data connection closed.\n");
                close(dcSocket); /* Close the data connection socket regardless of any errors that may have occurred. */
            }
            
            /* Implementing the recv command in the system */
            else if(strcmp(cmd, "recv") == 0){
                /* Try to establish a data connection even if there is an error to prevent the client from waiting indefinitely. */
                printf("Calling clntConnect to connect to the client.\n");
                status=clntConnect("10.3.200.17", &dcSocket);  /* Off-campus IP: "143.241.37.230" */
                if(status != 0){
                    /* When a data connection fails, send a response to the client and close the socket for the data connection. */
                    strcpy(replyMsg, "425 'recv' could not establish data connection. Closing data connection socket.\n");
                }else{
                    printf("Data connection to client successful.\n");
                    /* The data connection was established successfully. Verify that the user has logged in and provided a filename. */
                    if(argument[0] == NULL || strcmp(argument, "") == 0){
                        strcpy(replyMsg, "501 Invalid syntax. Use: \"send <filename>\". Closing Data connection.\n");
                    }
                    else if(isLoggedIn == LOGGED_IN){
                    
                        /* Make sure that the file specified can be read. */
                        filePtr = NULL;
                        filePtr = fopen(argument, "r");
                        if(filePtr == NULL){
                            strcpy(replyMsg, "550 The file \"");
                            strcat(replyMsg, argument);
                            strcat(replyMsg, "\" could not be opened. Closing data connection.");
                        }
                        else{
                            /* The file is readable, start sending its contents until the end is reached. */
                            ftpBytes = 0; /* set bytes to 0 */
                            printf("Sending file.\n");
                            do{
                                fileBytesRead = 0;
                                fileBytesRead = fread(ftpData, 1, 100, filePtr); /* The number of bytes read is returned by read error, making it impossible to distinguish it from a non-error situation. */
                                status = sendMessage(dcSocket, ftpData, fileBytesRead); /* Avoid modifying the file contents and refrain from adding a NULL element. */
                                ftpBytes = ftpBytes + fileBytesRead;
                            }while(!feof(filePtr) && status == OK); /* Terminate the file sending loop. */
                            sprintf(replyMsg, "226 Sent %d", ftpBytes);
                            strcat(replyMsg, " bytes. Closing data connection.");
                        }
                        fclose(filePtr); /* Close the file, regardless of whether it was successfully sent or not. */
                    }
                    else{
                        /* If the user is not logged in, discard any message received on the dcSocket. */
                        strcpy(replyMsg, "530 Not logged in. Closing data connection.\n");
                    }
                }
                close(dcSocket); /* Close the data connection regardless of whether there was an error or not. */
            }
            
            /* Send a response to the quit command. */
		    else if(strcmp(cmd, "quit") == 0){
            /* Inform the user that the service is closing data and control connections. */
			strcpy(replyMsg, "221 Service closing data and control connections.\n");

            /* Print a message indicating that the data connection socket is being closed. */
			printf("Closing data connection socket.\n");
			close (dcSocket);  /* Close server data connection socket */

            /* Print a message indicating that the control connection socket is being closed. */
			printf("Closing control connection socket.\n");
			close (ccSocket);  /* Close client control connection socket */

            /* Print a message indicating that the listen socket is being closed. */
			printf("Closing listen socket.\n");
			close(listenSocket);  /*close listen socket */

            /* Print a message indicating that the server FTP main process is exiting. */
			printf("Exiting from server ftp main. \n");

            /* Return the current status. */
			return (status);
		    }

            /* The input command is invalid and cannot be recognized. This is when the user is logged in. */
            else
			    strcpy(replyMsg, "502 Command not implemented. Use \"help\" for a list of valid commands.\n");
		    
        }
		
		/* The input command is invalid and cannot be recognized. This is when the user is NOT logged in */
		else{
			strcpy(replyMsg, "502 Command not implemented. Use \"help\" for a list of valid commands.\n");
		}
	
	
		
	    /*
 	     * ftp server sends only one reply message to the client for 
	     * each command received in this implementation.
	     */    

        /* Send a reply message to the client using the control connection socket. */
        /* Add 1 to the message length to include the NULL character, as strlen does not count the NULL character. */
	    status=sendMessage(ccSocket,replyMsg,strlen(replyMsg) + 1);
		
        /* Check if the status of the sendMessage function is less than 0 (indicating an error). */
	    if(status < 0)
	    {	
		break;  /* If there's an error, exit the while loop. */
	    }
	}	
	while(1);
    /* Free the memory allocated for the 'cmd' and 'argument' variables. */
	free(cmd);
    free(argument);
}


/*
 * svcInitServer
 *
 * Function to create a socket and to listen for connection request from client
 * using the created listen socket.
 *
 * Parameters
 * s        - Socket to listen for connection request (output)
 *
 * Return status
 *  OK                      - Successfully created listen socket and listening
 *  ER_CREATE_SOCKET_FAILED - socket creation failed
 */

int svcInitServer(int *s) {

	int sock;
    struct sockaddr_in svcAddr;
    int qlen;

    /* Create a socket endpoint */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("cannot create socket");
        return (ER_CREATE_SOCKET_FAILED);
    }

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(sock);
        return (ER_CREATE_SOCKET_FAILED);
    }

    /* Initialize memory of svcAddr structure to zero. */
    memset((char *)&svcAddr, 0, sizeof(svcAddr));

    /* Initialize svcAddr to have server IP address and server listen port#. */
    svcAddr.sin_family = AF_INET;
    svcAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    svcAddr.sin_port = htons(SERVER_FTP_PORT);

    /* Bind the listen socket number with server IP and port#. */
    if (bind(sock, (struct sockaddr *)&svcAddr, sizeof(svcAddr)) < 0) {
        perror("cannot bind");
        close(sock);
        return (ER_BIND_FAILED);
    }

    /* Set listen queue length to 1 outstanding connection request. */
    qlen = 1;

    /* Listen for connection requests from client ftp. */
    listen(sock, qlen);

    /* Store listen socket number to be returned in output parameter 's' */
    *s = sock;

    return (OK);
}

/*
 * clntConnect
 *
 * Copied from clientftp.c.
 *
 * This function creates a socket, associates the local client IP address and port with the socket, and establishes a connection to the client.
 * 
 *
 * Parameters
 * serverName	- IP address of client in dot notation (input)
 * s		    - Control connection socket number (output)
 *
 * Return status
 *	OK			- Successfully connected to the client
 *	ER_INVALID_HOST_NAME	- Invalid client name
 *	ER_CREATE_SOCKET_FAILED	- Cannot create socket
 *	ER_BIND_FAILED		- bind failed
 *	ER_CONNECT_FAILED	- connect failed
 */
int clntConnect (
	char *serverName, /* The IP address of the client in dotted decimal notation (input). */
	int  *s 		  /* Output parameter for the function that represents the socket number for the control connection. */
	)
{
	int sock;	/* Local variable to store the socket descriptor/identifier. */

	struct sockaddr_in clientAddress;  	/* The IP address of the local server as a string. */
	struct sockaddr_in serverAddress;	/* IP address of the client.*/
	struct hostent	   *serverIPstructure;	/* A host entry containing the server's IP address in binary format.*/

	/* Retrieve the binary IP address of the client from the client name (in dot notation). */
	if((serverIPstructure = gethostbyname(serverName)) == NULL)
	{
		printf("%s is unknown server. \n", serverName);
		return (ER_INVALID_HOST_NAME);  /* A return value indicating an error for a host name. */
	}

	/* Create a socket for the control connection. */
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("cannot create socket ");
		return (ER_CREATE_SOCKET_FAILED);	/* A return value indicating an error for a socket. */
	}

	/* Clear the memory of the server address structure. */
	memset((char *) &clientAddress, 0, sizeof(clientAddress));

	/* Set the IP address and port of the local server in the address structure. */
	clientAddress.sin_family = AF_INET;	/* The protocol family used for communication over the network. */
	clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);  /* The value of INADDR_ANY is 0, so by using it, the system is allowed to fill in the server IP address automatically. */
	clientAddress.sin_port = 0;  /* When the port number is set to 0, the system will automatically allocate a free port from 1024 to (64K -1) */

	/* Associate the socket with the local server IP address and port by binding it.*/
	if(bind(sock,(struct sockaddr *)&clientAddress,sizeof(clientAddress))<0)
	{
		perror("cannot bind");
		close(sock);
		return(ER_BIND_FAILED);	/* Binding of the socket was unsuccessful. */
	}


	/* Reset the memory of serverAddress to 0. */
	memset((char *) &serverAddress, 0, sizeof(serverAddress));

	/* Set the FTP client address in the serverAddress structure. */
	serverAddress.sin_family = AF_INET;
	memcpy((char *) &serverAddress.sin_addr, serverIPstructure->h_addr, 
			serverIPstructure->h_length);
	serverAddress.sin_port = htons(DATA_CONNECTION_PORT);

	/* Establish a connection to the FTP client. */
	if (connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
	{
		perror("Cannot connect to client ");
		close (sock); 	/* Close the socket used for the control connection. */
		return(ER_CONNECT_FAILED);  	/* error return */
	}


	/* Save the listen socket number and assign it to the output parameter 's'. */
	*s=sock;

	return(OK); /* successful return */
}  // end of clntConnect() */


/*
 * sendMessage
 *
 * Function to send specified number of octet (bytes) to client ftp
 *
 * Parameters
 * s		- Socket to be used to send msg to client (input)
 * msg  	- Pointer to character arrary containing msg to be sent (input)
 * msgSize	- Number of bytes, including NULL, in the msg to be sent to client (input)
 *
 * Return status
 *	OK		- Msg successfully sent
 *	ER_SEND_FAILED	- Sending msg failed
 */

int sendMessage(
	int    s,	/* socket to be used to send msg to client */
	char   *msg, 	/* buffer having the message data */
	int    msgSize 	/* size of the message/data in bytes */
	)
{
	int i;


	/* Print the message to be sent byte by byte as character */
	for(i=0; i<msgSize; i++)
	{
		printf("%c",msg[i]);
	}
	printf("\n");

	if((send(s, msg, msgSize, 0)) < 0) /* socket interface call to transmit */
	{
		perror("unable to send ");
		return(ER_SEND_FAILED);
	}

	return(OK); /* successful send */
}

/*
 * receiveMessage
 *
 * Function to receive message from client ftp
 *
 * Parameters
 * s		- Socket to be used to receive msg from client (input)
 * buffer  	- Pointer to character arrary to store received msg (input/output)
 * bufferSize	- Maximum size of the array, "buffer" in octent/byte (input)
 *		    This is the maximum number of bytes that will be stored in buffer
 * msgSize	- Actual # of bytes received and stored in buffer in octet/byes (output)
 *
 * Return status
 *	OK			- Msg successfully received
 *	R_RECEIVE_FAILED	- Receiving msg failed
 */


int receiveMessage (
	int s, 		/* socket */
	char *buffer, 	/* buffer to store received msg */
	int bufferSize, /* how large the buffer is in octet */
	int *msgSize 	/* size of the received msg in octet */
	)
{
	int i;

	*msgSize=recv(s,buffer,bufferSize,0); /* socket interface call to receive msg */

	if(*msgSize<0)
	{
		perror("unable to receive");
		return(ER_RECEIVE_FAILED);
	}

	/* Print the received msg byte by byte */
	for(i=0;i<*msgSize;i++)
	{
		printf("%c", buffer[i]);
	}
	printf("\n");

	return (OK);
}

