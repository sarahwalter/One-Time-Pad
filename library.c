#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "library.h"

/*
 * Return the min between the received numbers
 */
int
min(int a, int b)
{
	return a <= b ? a : b;		/* If a is less than b return a, else return b */
}
/*
 * Send a confirmation message through the socket
 */
int
send_confirmation(int socket_fd)
{
	if (send(socket_fd, "0", 2, 0) == -1) {		/* Send the message "0". It send two bytes for the character '0' and '\0' */
		perror("send");
		return -1;
	}
	return 0;
}
/*
 * Get a confirmation message through the socket
 */
int
get_confirmation(int socket_fd)
{
	char buffer[2];
	if (recv(socket_fd, buffer, 2, 0) == -1) {  /* Same as above but receiving */
		perror("recv");
		return -1;
	}
	return 0;
}
/*
 * Get the length of the next message 
 */
int
get_len(int socket_fd)
{
	char buffer[PACKET_SIZE];
	if (recv(socket_fd, buffer, PACKET_SIZE , 0) == -1) {		/* Receive */
		perror("recv");
		return -1;
	}
	return atoi(buffer);		/* Transform the string to an integer and return it */
}
/*
 * Send the length of the next message 
 */
int 
send_len(int socket_fd, int len)
{
	char buffer[PACKET_SIZE];
	int size;
	sprintf(buffer, "%d", len);		/* Transform the integer to a string and then send it */
	size = strlen(buffer);
	if (send(socket_fd, buffer, size + 1, 0) == -1) {
		perror("send");
		return -1;
	}
	return 0;
}
/*
 * Send through the socket the specified bytes that beging at buffer 
 */
int
send_all (int socket_fd, void *buffer, int size)
{
	int remaining, sent, total;			/* Remaining is the how much we need still send. Total is how much we have sent */
	remaining = size;
	total = 0;
	while (remaining > 0 && (sent = send(socket_fd, buffer + total,		/* While there is still info to send and process doesn't fail */
										min(remaining, PACKET_SIZE),
										0)) > 0) {
		remaining -= sent;			/* Upgrade the values */
		total+=sent;
	}
	if (remaining > 0) {			/* If we couldn't send all the bytes */
		perror("send");
		return -1;
	}
	return 0;
}
/*
 * Get the specified bytes through the socket and save to destination
 * or print it to stdout according to mode
 */
int
recv_all (int socket_fd, void *destination, int size)
{
	char buffer[PACKET_SIZE+1];
	int remaining, received, total;				/* Same as above but receiving */
	remaining = size,
	total = 0;
	while (remaining > 0 && (received = recv(socket_fd,
											destination + total,
											min(PACKET_SIZE,
											remaining), 0)) > 0) {
		remaining -= received;
		total+=received;
	}
	if (remaining > 0) {						/* If we couldn't receive all the bytes */
		perror("recv");
		return -1;
	}
	return 0;
}
int send_identifier(int sockfd, char *id){
	if(send(sockfd, id, 3, 0) < 0) {
		fprintf(stderr, "Error sending identifier\n");
		return -1;
	}
	return 0;
}
int get_identifier(int sockfd, char *id){
	if(recv(sockfd, id, 3, 0) <= 0) {
		fprintf(stderr, "Error receiving identifier\n");
		return -1;
	}
	return 0;
}
void error(const char *msg) { perror(msg); exit(0); } /* Error function used for reporting issues */


