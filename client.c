#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_NAME_LEN_MAX 255

int main(int argc, char *argv[]) {
	char server_name[SERVER_NAME_LEN_MAX + 1] = { 0 };
	char *filename;
	int server_port, socket_fd;
	struct sockaddr_in server_address;
	struct hostent *server_host;

	/* Get server name from command line arguments or stdin. */
	if (argc > 1) {
		strncpy(server_name, argv[1], SERVER_NAME_LEN_MAX);
	} else {
	        printf("Enter Server Name: ");
	        scanf("%s", server_name);
	}

	/* Get server port from command line arguments or stdin. */
	server_port = argc > 2 ? atoi(argv[2]) : 0;
	if (!server_port) {
	         printf("Enter Port: ");
	         scanf("%d", &server_port);
	}
	
	/* Get file name from command line arguments or stdin. */
	filename = argc > 3 ? argv[3] : " ";
	if (strcmp(filename," ")==0){
		printf("Enter file name: ");
		scanf("%s", filename);
	}
	
	FILE *fp = fopen(filename,"r");
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char server_message[2000];
	memset(server_message, '\0', sizeof(server_message));
	
	/* Get server host from server name. */
	server_host = gethostbyname(server_name);

	/* Create TCP socket. */
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	         perror("socket");
	         exit(1);
	}

	/* Initialise IPv4 server address with server host. */
	memset(&server_address, 0, sizeof server_address);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server_port);
	memcpy(&server_address.sin_addr.s_addr, server_host->h_addr, server_host->h_length);


	/* Connect to socket with server address. */
	if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof server_address) == -1) {
	 	perror("connect");
		exit(1);
	}
	fp = fopen(filename,"r");
	if (fp == NULL){
		printf("ERROR: Failed to open file");
		exit(1);
	}
	// sends filename to write to, to server
	if (send(socket_fd, filename, strlen(filename), 0) < 0){
		printf("Client ERROR: first message failed!");
		exit(1);
	}
	// receive ACK from server along with file name of reverse contents
	if (recv(socket_fd, server_message, sizeof(server_message), 0) < 0){
		printf("Client ERROR: first ACK failed!");
		exit(1);
	}

	// read line from file
	while ((read = getline(&line, &len, fp)) != -1){
		// send to server
		if (send(socket_fd, line, sizeof(line), 0) < 0){
			printf("Client ERROR: Failed to send line");
			exit(1);
		}
		if (recv(socket_fd, server_message, sizeof(server_message), 0) < 0){
			printf("Client ERROR: Failed to receive ACK from Server");
			exit(1);
		}
	}
	fclose(fp);
	close(socket_fd);
	return 0;
}
