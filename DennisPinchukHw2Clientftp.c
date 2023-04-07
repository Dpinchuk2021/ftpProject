/* 
 * Client FTP program
 *
 * NOTE: Starting homework #2, add more comments here describing the overall function
 * performed by server ftp program
 * This includes, the list of ftp commands processed by server ftp.
 *
 */

#include <stdlib.h>
#include <stdio.h>
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


/* Function prototypes */

int clntConnect(char *serverName, int *s);
int svcInitServer(int *s); /* Initialize a server and store the server socket number in the memory */
int sendMessage (int s, char *msg, int  msgSize);
int receiveMessage(int s, char *buffer, int  bufferSize, int *msgSize);


/* List of all global variables */

char userCmd[1024];	/* user typed ftp command line read from keyboard */
char *cmd;		/* ftp command extracted from userCmd */
char *argument;	/* argument extracted from userCmd */
char replyMsg[1024];    /* buffer to receive reply message from server */
char userCmdCopy[1024];

char ftpData[1024];		/* A buffer used to transmit or receive data to/from the client. */
int  ftpBytes      = 0; /* This variable is used to keep track of the total number of bytes transferred during the FTP process.*/
int  fileBytesRead = 0; /* The variable to store the number of bytes read by the fread function.*/
int  bytesReceived = 0; /* The size of a single FTP message in bytes received. */
FILE *filePtr;          /* This is a pointer to the temporary file that records the output of the command.*/


/*
 * main
 *
 * Function connects to the ftp server using clntConnect function.
 * Reads one ftp command in one line from the keyboard into userCmd array.
 * Sends the user command to the server.
 * Receive reply message from the server.
 * On receiving reply to QUIT ftp command from the server,
 * close the control connection socket and exit from main
 *
 * Parameters
 * argc		- Count of number of arguments passed to main (input)
 * argv  	- Array of pointer to input parameters to main (input)
 *		   It is not required to pass any parameter to main
 *		   Can use it if needed.
 *
 * Return status
 *	OK	- Successful execution until QUIT command from client 
 *	N	- Failed status, value of N depends on the function called or cmd processed
 */

int main(	
	int argc,
	char *argv[]
	)
{
	/* List of local varibale */

	int ccSocket;	/* Control connection socket - to be used in all client communication */
	int dcSocket;     /* Data connection socket - to be used in data transfer to and from server */
	int listensocket; /* Used to listen for connection requests to the dcSocket. */
	int msgSize;	/* size of the reply message received from the server */
	int status = OK;  /* Set status to ok */

	/*
	 * NOTE: without \n at the end of format string in printf,
         * UNIX will buffer (not flush)
	 * output to display and you will not see it on monitor.
 	 */
	printf("Started execution of client ftp\n");


	 /* Connect to client ftp*/
	printf("Calling clntConnect to connect to the server\n");	/* changed text */

	status=clntConnect("10.3.200.17", &ccSocket);
	if(status != 0)
	{
		printf("Connection to server failed, exiting main. \n");
		return (status);
	}

	/*
	 * Listen for a data connection.
	 * Copied and edited from serverftp.c.
	 */
	printf("Initialize client data connection to server (listen).\n");	/* Tells the user that the terminal is initializing the client data connection to the server. */

	status=svcInitServer(&listensocket);
	if(status != 0)
	{
		printf("Exiting client ftp due to svcInitServer returned error.\n");
		exit(status); /* If there is no data connection then it will return and error and exit. */
	}


	/* 
	 * Read an ftp command with argument, if any, in one line from user into userCmd.
	 * Copy ftp command part into ftpCmd and the argument into arg array.
 	 * Send the line read (both ftp cmd part and the argument part) in userCmd to server.
	 * Receive reply message from the server.
	 * until quit command is typed by the user.
	 */

	do
	{
		printf("my ftp> ");
		
		/* Reset the initial value of the first element in the command and argument array. */
		/* cmd      = NULL; */
		/* argument = NULL; */
		
		/* Obtain the input provided by the user and store it in the variable userCmd. */
		fgets(userCmd, sizeof(userCmd), stdin);

		/* Remove the newline character from userCmd */
		size_t len = strlen(userCmd);
		if (len > 0 && userCmd[len - 1] == '\n') {
    			userCmd[len - 1] = '\0';
		}
		
		/* Extract the command and argument from the userCmd string. */
		strcpy(userCmdCopy, userCmd);
		cmd = strtok(userCmdCopy, " ");
		/* argument = strtok(NULL, " "); */

		if(strcmp(cmd, "send") != 0 && strcmp(cmd, "recv") != 0){
			printf("Sending message on ccSocket.\n");

			/* Send the command entered by the user to the server.*/
			status = sendMessage(ccSocket, userCmd, strlen(userCmd)+1);
		}
		if(status != OK) /* If the status isn't okay then break the cycle loop. */
		{
		    break;
		}

		/* Receive reply message from the the server */
		/* status = receiveMessage(ccSocket, replyMsg, sizeof(replyMsg), &msgSize); */
		if(status != OK)
		{
		    break;
		}

		/* Begin the process of sending the data.*/
		if(strcmp(cmd, "send") == 0){
			printf("break1\n");
			if(argument[0] == NULL || strcmp(argument, "") == 0){
				printf("File argument not specified. Data connection will not be opened. No command sent.\n");
			}else{
				printf("break3\n");
				filePtr = NULL;
				printf("break3\n");
				filePtr = fopen(argument, "r");
				printf("break4\n");

				/* Verify if the specified file is readable. If not, don't send the command or establish a data connection. */
				if(filePtr == NULL){
				printf("Could not open specified file. Data connection will not be opened. No command sent.\n");
				}
			
				/* If the file is readable, send the command to the server and establish a data connection.*/
				else{
					printf("send: Sending message on ccSocket.\n");
					status = sendMessage(ccSocket, userCmd, strlen(userCmd)+1); /* Transmit the command over the command connection. */
					ftpBytes = 0; /* Set the byte count to zero. */
					printf("ftp client is waiting to accept data connection.\n");

					/* Wait for a connection request from the FTP server. */
					dcSocket = accept(listensocket, NULL, NULL);
	
					printf("Came out of dcSocket accept() function \n");
	
					if(dcSocket < 0)
					{
						perror("Cannot accept data connection: Closing listen socket.\n");
						close(dcSocket);  /* Close the listening socket. */
					}
					/* The data connection has been established successfully, and file transfer can begin. */
					else{
						printf("Data connection established with server. Sending file.\n");
				
						/* Transmit the data of the specified file. */
						do{
							printf("Top of send do loop. status: %d\n", status); /* prints a debug message indicating the start of a loop and the current status. */
							fileBytesRead = 0; /* This keep track of the number of bytes read from the file being transferred. */
							fileBytesRead = fread(ftpData, 1, 100, filePtr); /* A read operation may return an error and the number of bytes read cannot be used to distinguish it from a successful read.*/
							printf("Read packet from file complete.\n"); /* Print a message indicating that reading a packet from a file is complete. */
							status = sendMessage(dcSocket, ftpData, fileBytesRead); /* Sends a file data message using the sendMessage() function with ftpData buffer and fileBytesRead as arguments, and sets the status variable with the function's return value that indicates the success of the message transmission. */
							printf("Packet sent. sendMessage status: %d\n", status); /* Prints a message indicating that a packet has been sent using the sendMessage() function, and also displays the status of the function call. */
							ftpBytes = ftpBytes + fileBytesRead;
							printf("Bottom of do loop.\n"); /* The program has reached the end of a loop and prints out the message that it ended. */
						}while(!feof(filePtr) && status == OK); /* The sending loop has come to an end. */
					
						/* prints out the results and closes the data socket. */
						if(status != OK){
							perror("sendMessage returned not OK: Closing data connection.\n");
						}else{
							printf("\nReached EOF. %d bytes sent. Closing data connection.\n\n", ftpBytes);
						}
						fclose(filePtr);
						close(dcSocket);
					}
					printf("send: receiving message on ccSocket.\n"); /* Prints a message indicating that the program is currently receiving a message on the command connection socket. */
					status = receiveMessage(ccSocket, replyMsg, sizeof(replyMsg), &msgSize); /* The process of receiving a reply from the server.*/
				}
			}
		}
		/* Start the process of receiving data from the server. */
		else if(strcmp(cmd, "recv") == 0){
			/* Before sending the 'recv' command, verify that a argument is provided and that the specified file can be written to.*/
			filePtr = NULL;
			filePtr = fopen(argument, "w");

			/* Ensures that the specified file can be written to by the program, before sending the 'recv' command to the server. */
			if(filePtr == NULL){
				perror("Could not open/create specified file. Data connection will not be opened.\n"); /* if there is no file then print out an error */
			}
			/* Since the specified file is writable, the 'recv' command will be sent to the server and the client will wait for a data connection request from the server.*/
			else{
				printf("Sending recv command to server.\n");
				status = sendMessage(ccSocket, userCmd, strlen(userCmd)+1);
				if(status != OK)
				{
					break;
				}
				printf("ftp client is waiting to accept data connection.\n");

				/* Wait until a connection request is received from the FTP server. */
				
				dcSocket = accept(listensocket, NULL, NULL);
				printf("Came out of dcSocket accept() function \n");
				if(dcSocket < 0){	
					/*  The data connection has failed, and instructs to close both the specified file that was supposed to be written and the data connection. */
					perror("Cannot accept data connection: Closing data connection.\n");
					fclose(filePtr);
					close(dcSocket);
				}
				else{
					/* The data connection between the client and server was successfully established, and the client is now receiving data from the server and writing it to the specified file. */
					printf("Data connection established with server. Waiting to receive file.\n");
					ftpBytes = 0; /* Set the byte count to zero. */
					do{
						bytesReceived = 0;
						status = receiveMessage(dcSocket, ftpData, sizeof(ftpData), &bytesReceived);
						fwrite(ftpData, 1, bytesReceived, filePtr);
						ftpBytes = ftpBytes + bytesReceived;
					}while(bytesReceived > 0 && status == OK); /*  The end of a loop that reads data over a data connection socket. */
			
				fclose(filePtr); /* Close the file. */
				printf("Received %d bytes. Closing data connection.\n\n", ftpBytes);
				close(dcSocket);
				}
				status = receiveMessage(ccSocket, replyMsg, sizeof(replyMsg), &msgSize);
			}
		}

		/* The client receives a reply message from the server if expected, but if a read error occurs on the client-side and a "send" is not sent, then no reply is expected. */
		if(strcmp(cmd, "send") != 0 && strcmp(cmd, "recv") != 0){
			printf("Receiving message on ccSocket.\n");
			status = receiveMessage(ccSocket, replyMsg, sizeof(replyMsg), &msgSize);
		}
		if(status != OK)
		{
		    break;
		}
		printf("Bottom of main do loop.\n");
	}
	while (strcmp(cmd, "quit") != 0);

	printf("Closing control connection \n");
	close(ccSocket);  /* close control connection socket */

	printf("Exiting client main \n");

	return (status);

}  /* end main() */


/*
 * clntConnect
 *
 * Function to create a socket, bind local client IP address and port to the socket
 * and connect to the server
 *
 * Parameters
 * serverName	- IP address of server in dot notation (input)
 * s		- Control connection socket number (output)
 *
 * Return status
 *	OK			- Successfully connected to the server
 *	ER_INVALID_HOST_NAME	- Invalid server name
 *	ER_CREATE_SOCKET_FAILED	- Cannot create socket
 *	ER_BIND_FAILED		- bind failed
 *	ER_CONNECT_FAILED	- connect failed
 */


int clntConnect (
	char *serverName, /* server IP address in dot notation (input) */
	int *s 		  /* control connection socket number (output) */
	)
{
	int sock;	/* local variable to keep socket number */

	struct sockaddr_in clientAddress;  	/* local client IP address */
	struct sockaddr_in serverAddress;	/* server IP address */
	struct hostent	   *serverIPstructure;	/* host entry having server IP address in binary */


	/* Get IP address os server in binary from server name (IP in dot natation) */
	if((serverIPstructure = gethostbyname(serverName)) == NULL)
	{
		printf("%s is unknown server. \n", serverName);
		return (ER_INVALID_HOST_NAME);  /* error return */
	}

	/* Create control connection socket */
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("cannot create socket ");
		return (ER_CREATE_SOCKET_FAILED);	/* error return */
	}

	/* initialize client address structure memory to zero */
	memset((char *) &clientAddress, 0, sizeof(clientAddress));

	/* Set local client IP address, and port in the address structure */
	clientAddress.sin_family = AF_INET;	/* Internet protocol family */
	clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY is 0, which means */
						 /* let the system fill client IP address */
	clientAddress.sin_port = 0;  /* With port set to 0, system will allocate a free port */
			  /* from 1024 to (64K -1) */

	/* Associate the socket with local client IP address and port */
	if(bind(sock,(struct sockaddr *)&clientAddress,sizeof(clientAddress))<0)
	{
		perror("cannot bind");
		close(sock);
		return(ER_BIND_FAILED);	/* bind failed */
	}


	/* Initialize serverAddress memory to 0 */
	memset((char *) &serverAddress, 0, sizeof(serverAddress));

	/* Set ftp server ftp address in serverAddress */
	serverAddress.sin_family = AF_INET;
	memcpy((char *) &serverAddress.sin_addr, serverIPstructure->h_addr, 
			serverIPstructure->h_length);
	serverAddress.sin_port = htons(SERVER_FTP_PORT);

	/* Connect to the server */
	if (connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
	{
		perror("Cannot connect to server ");
		close (sock); 	/* close the control connection socket */
		return(ER_CONNECT_FAILED);  	/* error return */
	}


	/* Store listen socket number to be returned in output parameter 's' */
	*s=sock;

	return(OK); /* successful return */
}  // end of clntConnect() */

int svcInitServer (
	int *s 		/*This function returns the listen socket number. */
	)
{
	int sock; /*  An integer representing the socket file descriptor that will be created and returned by this function. */
	struct sockaddr_in svcAddr; /* A structure of type sockaddr_in that will be used to store the address and port of the server that the socket will connect to. */
	int qlen; /* An integer representing the maximum length of the queue of pending connections for the socket. */

	/* Create an endpoint for a socket. */
	if( (sock=socket(AF_INET, SOCK_STREAM,0)) <0)
	{
		perror("cannot create socket");
		return(ER_CREATE_SOCKET_FAILED);

		/* This code block creates a socket endpoint using the socket() function with the specified address family (AF_INET), socket type (SOCK_STREAM), and protocol (0). If the socket creation fails, an error message is printed using perror(), and an error code (ER_CREATE_SOCKET_FAILED) is returned. The socket file descriptor is stored in the sock variable. */
	}

	/* Set all the memory of the svcAddr structure to zero to initialize it. */
	memset((char *)&svcAddr,0, sizeof(svcAddr));

	/* Initialize the structure svcAddr with the IP address and the listening port number of the client. */
	svcAddr.sin_family = AF_INET;
	svcAddr.sin_addr.s_addr=htonl(INADDR_ANY);  	 /* The clients IP address. */
	svcAddr.sin_port=htons(DATA_CONNECTION_PORT);    /* The clients listen port number. */

	/* Associate the server IP address and port number with the listen socket.
	 * The function bind() belongs to the socket interface.
	 */
	if(bind(sock,(struct sockaddr *)&svcAddr,sizeof(svcAddr))<0)
	{
		perror("cannot bind");
		close(sock);
		return(ER_BIND_FAILED);	/* This means that the attempt to associate a socket with a specific address and port number was not successful. */
	}

	/* 
	 * Set the listen queue length to 1, allowing for one outstanding connection request.
	 */
	qlen=1; 


	/* The program waits for a connection request to come from the FTP server by calling the "listen" function, which is a non-blocking socket interface function that allows the client to continue executing while listening for incoming connections. */

	listen(sock,qlen);  /* Function call related to socket interface. */
	*s=sock;
	return(OK); 
}

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
	int s, 		/* socket to be used to send msg to client */
	char *msg, 	/*buffer having the message data */
	int msgSize 	/*size of the message/data in bytes */
	)
{
	int i;


	/* Print the message to be sent byte by byte as character */
	for(i=0;i<msgSize;i++)
	{
		printf("%c",msg[i]);
	}
	printf("\n");

	if((send(s,msg,msgSize,0)) < 0) /* socket interface call to transmit */
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
 *	ER_RECEIVE_FAILED	- Receiving msg failed
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


/*
 * clntExtractReplyCode
 *
 * Function to extract the three digit reply code 
 * from the server reply message received.
 * It is assumed that the reply message string is of the following format
 *      ddd  text
 * where ddd is the three digit reply code followed by or or more space.
 *
 * Parameters
 *	buffer	  - Pointer to an array containing the reply message (input)
 *	replyCode - reply code number (output)
 *
 * Return status
 *	OK	- Successful (returns always success code
 */

int clntExtractReplyCode (
	char	*buffer,    /* Pointer to an array containing the reply message (input) */
	int	*replyCode  /* reply code (output) */
	)
{
	/* extract the codefrom the server reply message */
   sscanf(buffer, "%d", replyCode);

   return (OK);
}  // end of clntExtractReplyCode()
