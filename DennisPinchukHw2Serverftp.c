/* 
 * server FTP program
 *
 * NOTE: Starting homework #2, add more comments here describing the overall function
 * performed by server ftp program
 * This includes, the list of ftp commands processed by server ftp.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define SERVER_FTP_PORT 5017
#define DATA_CONNECTION_PORT 5018


/* Error and OK codes */
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

char userCmd[1024];	    /* user typed ftp command line received from client */
char userCmdCopy[1024]; /* used for temporary String manipulation. */
char chgCmd[1024];      /* used to change a command into a UNIX-recognizable command */
char *cmd;   		    /* ftp command (without argument) extracted from userCmd */
char *argument;         /* argument (without ftp command) extracted from userCmd */
char replyMsg[1024];    /* buffer to send reply message to client */
char ftpData[4096];		/* buffer to send/receive file data to/from client */
int  ftpBytes      = 0; /* Used to count the total number of bytes transferred during ftp */
int  fileBytesRead = 0; /* The number of bytes read by fread */
int  bytesReceived = 0; /* The number of bytes received in a single ftp message. */

FILE *filePtr;                                       /* Used to point to the temporary file that logged command output */
int  bytesRead    = -1;                              /* The number of bytes read by fread() */
char fileData[1024];                                 /* Used to store the byte data obtained by fread(). Max size = 1024 bytes per in-class discussion */

/* Login Codes */
#define LOGGED_IN 1
#define LOGGED_OUT 0
#define NO_USER -1



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

	char *userList[]  = {"Dennis", "Eric", "Steven", "Mike"}; /* The list of valid user names */
	char *passList[]  = {"JackSparrow", "isSunny46", "graygoose", "bluelabel45"};            /* The list of passwords associated with the index of userList */
	int  isLoggedIn   = -1;                              /* -1 (default), 0 if the User is not logged in, 1 otherwise. */
	int  userIndex    = -1;                              /* The index of userList that identifies the current user. See Login Codes. */



	/*
	 * NOTE: without \n at the end of format string in printf,
         * UNIX will buffer (not flush)
	 * output to display and you will not see it on monitor.
	*/
	printf("Started execution of server ftp\n");


	 /*initialize server ftp*/
	printf("Initialize ftp server\n");	/* changed text */

	status=svcInitServer(&listenSocket);
	if(status != 0)
	{
		printf("Exiting server ftp due to svcInitServer returned error\n");
		exit(status);
	}


	printf("ftp server is waiting to accept connection\n");

	/* wait until connection request comes from client ftp */
	ccSocket = accept(listenSocket, NULL, NULL);

	printf("Came out of accept() function \n");

	if(ccSocket < 0)
	{
		perror("cannot accept connection:");
		printf("Server ftp is terminating after closing listen socket.\n");
		close(listenSocket);  /* close listen socket */
		return (ER_ACCEPT_FAILED);  // error exist
	}

	printf("Connected to client, calling receiveMsg to get ftp cmd from client \n");


	/* Receive and process ftp commands from client until quit command.
 	 * On receiving quit command, send reply to client and 
         * then close the control connection socket "ccSocket". 
	 */
	do
	{
	    /* Receive client ftp commands until */
 	    status=receiveMessage(ccSocket, userCmd, sizeof(userCmd), &msgSize);
	    if(status < 0)
	    {
		printf("Receive message failed. Closing control connection \n");
		printf("Server ftp is terminating.\n");
		break;
	    }
	    
	    /* Debugging Purposes */
	    int bytesSent;
    	    char replyMsg[] = "Message received.\n";
          bytesSent = send(ccSocket, replyMsg, strlen(replyMsg), 0); /* Send the reply message to the client. */
    	    printf("Sent reply: %s", replyMsg);
          printf("bytesSent: %d\n", bytesSent);


/*
 * Starting Homework#2 program to process all ftp commands must be added here.
 * See Homework#2 for a list of FTP commands to implement.
 */

	    /* Reinitialize the first element of the command and argument. */

	    char *cmd = NULL;
	    char *argument = NULL;

	    printf("Message received. status: %d.\n", status); /* The client is informed that their message has been received by the program. */

	    if (userCmd == NULL) {
    			printf("userCmd is NULL.\n"); /* There is no valid command or argument will be printed */
	    } else if (strcmp(userCmd, "") == 0) {
    			printf("userCmd is the empty set.\n"); /* otherwise, the program will print out an empty set */
	    }

	    printf("cmd and arg initialized.\n"); /* Output a message indicating that the command and argument have been initialized */
	    printf("cmd: %s.\n", cmd);
	    printf("arg: %s.\n", argument);

	    /* "Extract the command and argument from the 'userCmd' variable using strtok */
	    strcpy(userCmdCopy, userCmd);
	    cmd = strtok(userCmdCopy, " ");
	    argument = strtok(NULL, " ");

	    /* Output a message indicating that the server has received the command */

	    printf("Received command: %s.\n", userCmd);
	    printf("cmd: %s. \n", cmd); 
	    printf("arg: %s.\n", argument);

	    /* The initial conditional statement to assist with handling the command in the server. The beginning of 1 out of 13 of these commands. */
	    /* Establish the user command if the user is authenticated/authorized*/   

	    if(strcmp(cmd, "help") == 0) {
		strcpy(replyMsg, "214 user stat mkdir rmdir dele cd help ls pwd pass \n");
		}
		else if(strcmp(cmd, "user") == 0) {
    			isLoggedIn = NO_USER;
    			userIndex = -1;

    			if(argument[0] == NULL || strcmp(argument, "") == 0) {
        			strcpy(replyMsg, "501 No username given. Usage: \"user <username>\".\n");
        			userIndex = -1;
    			}else {
        			int i = 0;
        			int numUsers = sizeof(userList) / sizeof(*userList);
        			for(; i < numUsers; i++) {
            			if(strcmp(userList[i], argument) == 0) {
                			userIndex = i;
                			isLoggedIn = LOGGED_OUT;
                			strcpy(replyMsg, "331 User name good, need password.\n");
                			break;
            			}
        		}
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

	    else if(strcmp(cmd, "pass") == 0){
		/* The program will inform the user of a syntax error if they fail to provide a password or an "empty" password. */
		if(argument[0] == NULL || strcmp(argument, "") == 0) {
			strcpy(replyMsg, "501 Syntax error. Use: \"pass <password>\". \n");
		}
		/* If the user fails to send a valid user command, inform them that the command has failed. */
		else if(isLoggedIn == NO_USER || userIndex < 0){
				isLoggedIn = LOGGED_IN; 
				strcpy(replyMsg, "503 Bad sequence of commands. Use \"user <username>\" first.\n");
		}
		/* If a user/password combination is matched successfully. It's sent to the isLoggedIn and will send a reply. */
		else if(strcmp(passList[userIndex], argument) == 0) {
			isLoggedIn = LOGGED_IN; /* indicating that a user has successfully logged into the server. */ 
			strcpy(replyMsg, "230 User logged in, proceed.\n");
		}

		/* If the user provides an invalid command, error messages are displayed, but the 'userIndex' variable remains unchanged, and the 'isLoggedIn' variable is set to 'LOGGED_OUT' to ensure that the user is logged out of the server. */
		else{
			isLoggedIn = LOGGED_OUT;
			if(userIndex >= 0) {
				strcpy(replyMsg, "530 Invalid password for ");
				strcat(replyMsg, userList[userIndex]);
				strcat(replyMsg, ". Use \"pass\" command to try it again, or \"user\" command to switch to new users.\n");
			}
			/* The user will be informed of an error and the system will be shut down, although this is unlikely to happen and is merely a precautionary measure. */
			else {
				strcpy(replyMsg, "421 An error occurred. Please contact the system admin.");
				strcpy(cmd, "quit");
				}
			}
	    }

	    /* Implmenting the stat command in the system */
	    /* Indicates that the transfer mode for the FTP connection is set to ASCII mode */
	    else if(strcmp(cmd, "stat") == 0 || strcmp(cmd, "status") == 0) {
			strcpy(replyMsg, "211 Transfer mode is ASCII. \n");
	    }

	    /* Implementing the mkdir command in the system */
	    /* checks if the user command is either "mkdir" or "Mkd". If the user command matches either of these strings, then the code checks if the 'argument' variable is empty or not. */
	     else if (strcmp(cmd, "mkdir") == 0 || strcmp(cmd, "Mkd") == 0) {
			if (argument[0] == NULL || strcmp(argument, "") == 0) {
        		strcpy(replyMsg, "501 Syntax error. Use: \"mkdir <directory_path>\".\n");
    		}
    		else if (isLoggedIn == LOGGED_IN) {
        		if (strcmp(cmd, "mkd") == 0) {
            	strcpy(chgCmd, "mkdir ");
            	strcat(chgCmd, argument);
            	status = system(chgCmd);
        	} else {
            	status = system(userCmd);
        	}
        	if (status == 0) {
            	strcpy(replyMsg, "257 Created the directory \"");
            	strcat(replyMsg, argument);
            	strcat(replyMsg, "\".\n");
        	} else {
            	strcpy(replyMsg, "500 Unable to create directory \"");
            	strcat(replyMsg, argument);
            	strcat(replyMsg, "\".\n");
        		}
    		} 
		else {
        		strcpy(replyMsg, "530 Not logged in.\n");
    		}
	}

	else if (strcmp(cmd, "rmdir") == 0 || strcmp(cmd, "Mkd") == 0) {
    		if (argument[0] == NULL || strcmp(argument, "") == 0) {
        	strcpy(replyMsg, "501 Syntax error. Use: \"rmdir <directory_path>\".\n");
    }
    	else if (isLoggedIn == LOGGED_IN) {
        	if (strcmp(cmd, "rmd") == 0) {
            	strcpy(chgCmd, "rmdir ");
            	strcat(chgCmd, argument);
            	status = system(chgCmd);
        	} else {
            	status = system(userCmd);
        	}
        	if (status == 0) {
            	strcpy(replyMsg, "250 Removed the directory \"");
            	strcat(replyMsg, argument);
            	strcat(replyMsg, "\".\n");
        	} else {
            	strcpy(replyMsg, "500 Unable to remove \"");
            	strcat(replyMsg, argument);
            	strcat(replyMsg, "\".\n");
        	}
    	} else {
        	strcpy(replyMsg, "530 Not logged in.\n");
    		}
	}

	    /* Allowing the cd command in the system */
	    else if(strcmp(cmd, "cd") == 0){
			if(isLoggedIn == LOGGED_IN){
				/* Home is the login directory for the user who run this program. */
				if(argument[0] == NULL || strcmp(argument, "") == 0){
					status = chdir(getenv("HOME"));
					strcpy(argument, "HOME directory");
				}else{
					status = chdir(argument);
				}
				/* If the cd command is passed and worked there will be a message that states that it worked. */
				if(status == 0){
					strcpy(replyMsg, "250 Current working directory changed to \"");
					strcat(replyMsg, argument);
					strcat(replyMsg, "\".\n");
				}
				else{
					strcpy(replyMsg, "500 Unable to change to the specified directory.\n");
				} /* If the directory does not exist then pass along that message. */
			}
			else{
				strcpy(replyMsg, "530 Not logged in.\n");
			}
		}

	   	/* Allowing the pwd command in the system */
		else if(strcmp(cmd, "pwd") == 0){
			printf("Entered 'pwd' command block.\n"); /* Testing */
			if(isLoggedIn == LOGGED_IN){
				status = system("pwd > ./serverftp_temp");
				if(status == 0){
					filePtr = fopen("serverftp_temp", "r");
					if(filePtr == NULL){
						strcpy(replyMsg, "500 Unable to display current working directory (filePtr missing).\n");
					}
					/* If the file doesn't exist or couldn't be opened, notify the user of the failure. */
					else{
						bytesRead = fread(fileData, 1, 1023, filePtr); /* Leave one byte for null-terminator */
						/* If the file is empty, something went wrong. Notify the user of the failure. */
						if(bytesRead <= 0){
							strcpy(replyMsg, "500 Unable to display current directory (read error).\n");
						}
						/* Read output file. */
						else{
							fileData[bytesRead] = '\0'; /* NULL-terminate the file data */
							strcpy(replyMsg, "257 ");
							strcat(replyMsg, fileData);
							strcat(replyMsg, "\n");
						} 
						/* File read successfully. Send the user the server response. */
					}
					fclose(filePtr);
					system("rm ./serverftp_temp");
					/* Clean up the file data */
				}
				else{
					strcpy(replyMsg, "500 Unable to display current directory (can not process command).\n");
				}
				/* The output file could not be created. Notify the user that it failed */
			}
			else{
				strcpy(replyMsg, "530 Not logged in.\n");
			}
		}
	   	/* Allowing the pwd command in the system */
		else if(strcmp(cmd, "ls") == 0){
			if(isLoggedIn == LOGGED_IN){
				status = system("ls > ./serverftp_temp");
				if(status == 0){
					filePtr = fopen("./serverftp_temp", "r");
					/* If the file doesn't exist or couldn't be opened, notify the user of the failure. */
					if(filePtr == NULL){
						strcpy(replyMsg, "451 Unable to display directory contents (filePtr missing).\n");
					}
					/* Read the output file. */
					else{
						bytesRead = fread(fileData, 1, 1024, filePtr);
						/* Unable to read file. Notify user of failure. */
						if(bytesRead < 0){
							strcpy(replyMsg, "451 Unable to display directory contents (read error).\n");
						}
						/* File read successful. Send user the server response. */
						else{
							if(bytesRead == 0){
								strcpy(fileData, "The current directory is empty.");
							}
							fileData[bytesRead] = NULL; /*NULL-terminate the file data */
							strcpy(replyMsg, "\n");
							strcat(replyMsg, fileData);
							strcat(replyMsg, "\n250 Requested file action okay, completed.\n");
						}
					}
					/* Clean up */
					fclose(filePtr);
					system("rm ./serverftp_temp");
				}
				/* Output file could not be created. Therefore, notify the user of failure */
				else{
					strcpy(replyMsg, "451 Unable to display directory contents (can not process command).\n");
				}
			}
			else{
				strcpy(replyMsg, "530 Not logged in.\n");
			}
		}
		/* Allowing the ls command in the system */
		else if(strcmp(cmd, "ls") == 0){
			if(isLoggedIn == LOGGED_IN){
				status = system("ls > ./serverftp_temp");
				if(status == 0){
					filePtr = fopen("./serverftp_temp", "r");
					/* Notify the user of a failure if the file either does not exist or could  */
					if(filePtr == NULL){
						strcpy(replyMsg, "451 Unable to display directory contents (filePtr missing).\n");
					}
					/* Reads the output file */
					else{
						bytesRead = fread(fileData, 1, 1024, filePtr);
						/* Notify the user of a failure to read the file. */
						if(bytesRead < 0){
							strcpy(replyMsg, "451 Unable to display directory contents (read error).\n");
						}
						/* The file was successfully read. Send the server response to the user.*/
						else{
							if(bytesRead == 0){
								strcpy(fileData, "The current directory is empty.");
							}
							fileData[bytesRead] = NULL; /*Terminate the file data with NULL.*/
							strcpy(replyMsg, "\n");
							strcat(replyMsg, fileData);
							strcat(replyMsg, "\n250 Requested file action okay, completed.\n");
						}
					}
					/* Perform the necessary cleanup. */
					fclose(filePtr);
					system("rm ./serverftp_temp");
				}
				/* Notify the user of a failure to create the output file. */
				else{
					strcpy(replyMsg, "451 Unable to display directory contents (can not process command).\n");
				}
			} /* State that the user isn't logged in. */
			else{
				strcpy(replyMsg, "530 Not logged in.\n");
			}
		}
		
		/* Allowing the send command in the system */
		else if(strcmp(cmd, "send") == 0){
		
			/* Attempt a data connection even in the presence of errors to avoid the client from continuously listening. */
			printf("Calling clntConnect to connect to the client.\n");
			status=clntConnect("192.168.200.230", &dcSocket);  /* Off-campus IP: "143.241.37.230" */
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
		
		/* Allowing the recv command in the system */
		else if(strcmp(cmd, "recv") == 0){
			/* Try to establish a data connection even if there is an error to prevent the client from waiting indefinitely. */
			printf("Calling clntConnect to connect to the client.\n");
			status=clntConnect("192.168.200.230", &dcSocket);  /* Off-campus IP: "143.241.37.230" */
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
					strcpy(replyMsg, "530 Not logged in. Closing data connection.\n");
				}
			}
			close(dcSocket); /* Close the data connection regardless of whether there was an error or not. */
		}
		
		
		/* Send a response to the quit command. */
		else if(strcmp(cmd, "quit") == 0){
			strcpy(replyMsg, "221 Service closing data and control connections.\n");
		}
		
		/* The input command is invalid and cannot be recognized. */
		else{
			strcpy(replyMsg, "502 Command not implemented. Use \"help\" for a list of valid commands.\n");
		}
		
	    /*
 	     * ftp server sends only one reply message to the client for 
	     * each command received in this implementation.
	     */
	    status=sendMessage(ccSocket,replyMsg,strlen(replyMsg) + 1);	/* Added 1 to include NULL character in */
				/* the reply string strlen does not count NULL character */
	    if(status < 0)
	    {
		break;  /* exit while loop */
	    }
	}
	while(strcmp(cmd, "quit") != 0);
	
	printf("Closing data connection socket.\n");
	close (dcSocket);  /* Close server data connection socket */

	printf("Closing control connection socket.\n");
	close (ccSocket);  /* Close client control connection socket */

	printf("Closing listen socket.\n");
	close(listenSocket);  /*close listen socket */

	printf("Exiting from server ftp main. \n");

	return (status);
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


