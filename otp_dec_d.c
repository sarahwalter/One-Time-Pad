#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "library.h"

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, pid, count, lmsg, lkey, i, status, yes;
	int childs[5];
	socklen_t sizeOfClientInfo;
	char buffer[256];
	char *key;
	char *msg;
	struct sockaddr_in serverAddress, clientAddress;
	if (argc != 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } /* Check usage & args */
	for(i = 0; argv[1][i] != '\0'; i++)
		if(argv[1][i]<'0' || argv[1][i]>'9') {
			fprintf(stderr, "Port must be a positive number\n");
			return -1;
		}
	/* Set up the address struct for this process (the server) */
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); /* Clear out the address struct */
	portNumber = atoi(argv[1]); /* Get the port number, convert to an integer from a string */
	serverAddress.sin_family = AF_INET; /* Create a network-capable socket */
	serverAddress.sin_port = htons(portNumber); /* Store the port number */ 
	serverAddress.sin_addr.s_addr = INADDR_ANY; /* Any address is allowed for connection to this process */

	/* Set up the socket */
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); /* Create the socket */
	if (listenSocketFD < 0) error("ERROR opening socket");
	yes = 1;
	setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	/* Enable the socket to begin listening */
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) /* Connect socket to port */
		error("ERROR on binding");
	listen(listenSocketFD, 100); /* Flip the socket on - it can now receive up to 5 connections */
	count = 0;
	for(;;) {
		/* Accept a connection, blocking if one is not available until one connects */
		sizeOfClientInfo = sizeof(clientAddress); /* Get the size of the address for the client that will connect */
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); /* Accept */
		if (establishedConnectionFD < 0){
			fprintf(stderr, "ERROR on accept\n");
			continue;
		}
		if(count < 5) {
			if((pid = fork()) == 0) {
				memset(buffer, '\0', 256);
				if (get_identifier(establishedConnectionFD, buffer) >= 0) {	/* Getting identifier */
					send_identifier(establishedConnectionFD, "DD");			/* Sending identifier */
					if(strcmp(buffer, "CD") == 0) {							/* Checking identifier */
						if((lmsg = get_len(establishedConnectionFD)) < 0){	/* Getting length of the message */
							fprintf(stderr, "ERROR receiving from socket\n");
							close(establishedConnectionFD);
							return -1;
						}
						msg = (char*)malloc(sizeof(char)*lmsg);				/* Allocating memory */
						/* Error handling */
						if(msg == NULL){
							fprintf(stderr, "Error allocating memory\n");
							return -1;
						}
						if(send_confirmation(establishedConnectionFD) < 0 || recv_all(establishedConnectionFD, msg, lmsg) < 0){	//Confirm and receive the message
							fprintf(stderr, "ERROR receiving from socket\n");
							close(establishedConnectionFD);
							return -1;
						}
						
						if(send_confirmation(establishedConnectionFD) < 0 || (lkey = get_len(establishedConnectionFD)) < 0){	//Get the key
							fprintf(stderr, "ERROR receiving from socket\n");
							close(establishedConnectionFD);
							return -1;
						}
						key = (char*)malloc(sizeof(char)*lkey);
						if(key == NULL) {
							fprintf(stderr, "Error allocating memory\n");
							return -1;
						}
						if(send_confirmation(establishedConnectionFD) < 0 || recv_all(establishedConnectionFD, key, lkey) < 0){
							fprintf(stderr, "ERROR receiving from socket\n");
							close(establishedConnectionFD);
							return -1;
						}
						if(lkey >= lmsg) {					/* Checking compatiblity */
							for(i = 0; i < lmsg-1; i++) {	/* Translate into numbers and decipher the message */
								if(msg[i] == ' ')
									msg[i] = 26;
								else
									msg[i] = msg[i] - 'A';
								if(key[i] == ' ')
									key[i] = 26;
								else
									key[i] = key[i] - 'A';
								msg[i] = msg[i] - key[i];
								if(msg[i] < 0)
									msg[i] += 27;
							}
							for(i = 0; i < lmsg - 1; i++) {		/* Convert to characters */
								if(msg[i] == 26)
									msg[i] = ' ';
								else
									msg[i] = msg[i] + 'A';
							}
						}
						else {
							msg[0] = '\0';
						}
						if(send_all(establishedConnectionFD, msg, lmsg) < 0) {		/* Send the message */
							fprintf(stderr, "ERROR receiving from socket\n"); 
							close(establishedConnectionFD);
							return -1;
						}
						close(establishedConnectionFD); /* Close the existing socket which is connected to the client */
						return 0;
					}
					else {
						fprintf(stderr, "Connection received from an unexpected program\n");
						close(establishedConnectionFD); /* Close the existing socket which is connected to the client */
						return -1;
					}
				}
				else {
					fprintf(stderr, "ERROR reading from socket\n");
					close(establishedConnectionFD); /* Close the existing socket which is connected to the client */
					return -1;
				}
			}
			else {
				if(pid < 0) {
					perror("fork");
				}
				else {
					for(i = 0; i < 5; i++)
						if(childs[i] == -1){
							childs[i] = pid;
							count++;
							break;
						}
				}
			}
		}
		else {
			fprintf(stderr, "There are 5 active connections\n");
		}
		for(i = 0; i < 5; i++)
			if(childs[i] != -1)
				if(childs[i] == waitpid(childs[i],&status, WNOHANG) && WIFEXITED(status)) {
						childs[i] = -1;
						count --;
				}
	}
	close(listenSocketFD); /* Close the listening socket */
	return 0; 
}

