#ifndef LIBRARY_H_INCLUDED
#define LIBRARY_H_INCLUDED
#define PACKET_SIZE 1024

int min(int a, int b);
int send_confirmation(int socket_fd);
int get_confirmation(int socket_fd);
int get_len(int socket_fd);
int send_len(int socket_fd, int len);
int send_all(int socket_fd, void *buffer , int size);
int recv_all(int socket_fd, void *destination , int size);
int send_len(int socket_fd, int len);
int get_len(int socket_fd);
int send_identifier(int sockfd, char *id);
int get_identifier(int sockfd, char *id);
void error(const char *msg);

#endif // H_INCLUDED
