#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "library.h"


int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead, i, cipherlength, keylength;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char *buffer;
	char *key;
	char *ciphertext;
	char tmp;
	int sizekey;
	int sizetext;
    FILE *ciphertextf, *keyf;
	if (argc < 4) { fprintf(stderr,"USAGE: %s ciphertext key port\n", argv[0]); exit(0); } /* Check usage & args */
	for(i = 0; argv[3][i] != '\0'; i++)
		if(argv[3][i]<'0' || argv[3][i]>'9'){
			fprintf(stderr, "Port must be a positive number\n");
			return -1;
		}
	/* Set up the server address struct */
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); /* Clear out the address struct */
	portNumber = atoi(argv[3]); /* Get the port number, convert to an integer from a string */
	serverAddress.sin_family = AF_INET; /* Create a network-capable socket */
	serverAddress.sin_port = htons(portNumber); /* Store the port number */
	serverHostInfo = gethostbyname("localhost"); /* Convert the machine name into a special form of address */
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); /* Copy in the address */

	/* Set up the socket */
	socketFD = socket(AF_INET, SOCK_STREAM, 0); /* Create the socket */
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	/* Connect to server */
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){ /* Connect socket to address */
		fprintf(stderr,"CLIENT: ERROR connecting\n");
		close(socketFD);
		return 2;
	}
	ciphertextf = fopen(argv[1], "r");									/* Opening the file and allocating memory */
	/* Error reporting */
	if(ciphertextf == NULL) {
		fprintf(stderr, "Error with %s file\n", argv[1]);
		close(socketFD);
		return -1;
	}
	keyf = fopen(argv[2], "r");
	if(keyf == NULL) {
		fprintf(stderr, "Error with %s file\n", argv[2]);
		close(socketFD);
		return -1;
	}
	sizetext = 1024;
	sizekey = 1024;
	ciphertext = (char*)malloc(sizeof(char)* sizetext);
	if(ciphertext == NULL){
		fprintf(stderr, "Error allocating memory\n");
		close(socketFD);
		return -1;
	}
	i = 0;
	while(fscanf(ciphertextf,"%c", &tmp) != EOF){						/* Read the files */
		if(!(tmp == ' ' || (tmp >= 'A' && tmp <= 'Z') || tmp == '\n')){
			fprintf(stderr,"Ilegal characters in the ciphertext\n");
			close(socketFD);
			return -1;
		}
		if(tmp == '\n'){
			
			break;
		}
		if(i == sizetext -1) {											/* Allocate enough memory for te buffers */
			sizetext *= 2;	
			ciphertext = (char*)realloc(ciphertext, sizeof(char)*sizetext);	
			if(ciphertext == NULL){
				fprintf(stderr, "Error allocating memory\n");
				close(socketFD);
				return -1;
			}
		}
		ciphertext[i] = tmp;
		i++;
	}
	fclose(ciphertextf);
	cipherlength = i;
	ciphertext[i] = '\0';
	key = (char*)malloc(sizeof(char)* sizekey);
	/* Error reporting */
	if(key == NULL){
		fprintf(stderr, "Error allocating memory\n");
		close(socketFD);
		return -1;
	}
	i = 0;
	while(fscanf(keyf,"%c", &tmp) != EOF){
		if(!(tmp == ' ' || (tmp >= 'A' && tmp <= 'Z') || tmp == '\n')){
			fprintf(stderr,"Illegal characters in the key\n");
			close(socketFD);
			return -1;
		}
		if(tmp == '\n'){
			break;
		}
		if(i == sizekey -1) {
			sizekey *= 2;
			key = (char*)realloc(key, sizeof(char)*sizekey);
			if(key == NULL){
				fprintf(stderr, "Error allocating memory\n");
				close(socketFD);
				return -1;
			}
		}
		key[i] = tmp;
		i++;
	}
	fclose(keyf);
	keylength = i;
	key[i] = '\0';
	buffer = (char*)malloc(sizeof(char)*(cipherlength + 1));
	/* Error reporting */
	if(buffer == NULL){
		fprintf(stderr, "Error allocating memory\n");
		close(socketFD);
		return -1;
	}
	if(keylength < cipherlength) {														/* Checking compatibility */
		fprintf(stderr, "The length of the key and ciphertext are not compatible\n");
		close(socketFD);
		return 1;
	}
	if(send_identifier(socketFD,"CD") < 0 || get_identifier(socketFD, buffer) < 0) {	/* Sending and getting identifier */
		fprintf(stderr, "ERROR receiving from socket\n");
		close(socketFD);
		return -1;
	}
	if(strcmp(buffer, "DD") != 0) {														/* Checking identifier */
		close(socketFD);
		fprintf(stderr, "The connected service is not otp_dec_d\n");
		return -1;
	}
	if(send_len(socketFD, cipherlength +1) < 0 || get_confirmation(socketFD) < 0)		/* Sending text and key */
		error("CLIENT: ERROR sending to socket");
	if(send_all(socketFD, ciphertext, cipherlength + 1) < 0 || get_confirmation(socketFD) < 0) {
		close(socketFD);
		error("ERROR getting confirmation");
	}
	if(send_len(socketFD, keylength +1) < 0 || get_confirmation(socketFD) < 0){
		close(socketFD);
		error("CLIENT: ERROR sending to socket");
	}
	if(send_all(socketFD, key, keylength + 1) < 0) {
		close(socketFD);
		error("CLIENT: ERROR sending to socket");
	}
	if(recv_all(socketFD, buffer, cipherlength + 1) < 0){
		close(socketFD);
		error("CLIENT: ERROR reading from socket");
	}
	if(strlen(buffer) > 0)
		printf("%s\n", buffer);
	else
		fprintf(stderr,"CLIENT: ERROR in the message");
	close(socketFD); /* Close the socket */
	free(key);
	free(buffer);
	free(ciphertext);
	return 0;
}
