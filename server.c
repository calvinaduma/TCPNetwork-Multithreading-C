#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define BACKLOG 10

int count = 0;

typedef struct pthread_arg_t {
	int new_socket_fd;
    struct sockaddr_in client_address;
	int num;
} pthread_arg_t;

/* Thread routine to serve connection to client. */
void *pthread_routine(void *arg){
	FILE *fp;

	/* gets arguments from pthread function */
	pthread_arg_t *pthread_arg = (pthread_arg_t*)arg;
	int new_socket_fd = pthread_arg->new_socket_fd;
	struct sockaddr_in client_address = pthread_arg->client_address;
	int num = pthread_arg->num;
	free(arg);

	char buff[2000];
	bzero(buff, 2000);
	char *fileName = "file";
	
	int i=0;
	char *charNum;
	char *reversedLine;

	/* creates file name with a numbner at end */
	asprintf(&charNum, "%d", count);
	count++;
	strcat(fileName,charNum);
	printf("%s\n",fileName);
	fp = fopen(fileName,"w");

	/* Reads fisrt input from client */
	while(1){
		bzero(buff, 2000);
		// first message)
		if(pthread_arg->num==0){
			recv(new_socket_fd, buff, sizeof(buff),0);
			send(new_socket_fd, "ACK", 3, 0);
			send(new_socket_fd, fileName, sizeof(fileName), 0);
			pthread_arg->num = 1;
		} else {
			recv(new_socket_fd, buff, sizeof(buff),0);
			reversedLine = buff;
			strrev(reversedLine);
			send(new_socket_fd, "ACK", 3, 0);
			/* writes reversed line to file */
			for (i=0; reversedLine[i]!='\n';i++) fputc(reversedLine[i], fp);
		}
	}
	fclose(fp);
	close(new_socket_fd);
	return NULL;
}

int main(int argc, char *argv[]) {
	int server_port, socket_fd, new_socket_fd;
	struct sockaddr_in server_address;
	pthread_attr_t pthread_attr;
	pthread_arg_t *pthread_arg;
	pthread_t pthread;
	socklen_t client_address_len;

    /* Get port from command line arguments or stdin. */
    server_port = argc > 1 ? atoi(argv[1]) : 0;
    if (!server_port) {
        printf("Enter Port: ");
        scanf("%d", &server_port);
    }

    /* Initialise IPv4 address. */
    memset(&server_address, 0, sizeof server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    /* Create TCP socket. */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    /* Bind address to socket. */
    if (bind(socket_fd, (struct sockaddr *)&server_address, sizeof server_address) == -1) {
        perror("bind");
        exit(1);
    }

    /* Listen on socket. */
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

	/* Initialize pthread attribute to create detached threads */
	if (pthread_attr_init(&pthread_attr) != 0){
		printf("Server ERROR: pthread_attr_init");
		exit(1);
	}

	if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED) != 0){
		printf("Server ERROR: pthread_attr_setdetachstate");
		exit(1);
	}

	while (1){
		pthread_arg = (pthread_arg_t*)malloc(sizeof(*pthread_arg));
		if (!pthread_arg){
			printf("Server ERROR: malloc");
			continue;
		}

		/* Accept Connection from client */
		client_address_len = sizeof(pthread_arg->client_address);
		new_socket_fd = accept(socket_fd, (struct sockaddr*)&pthread_arg->client_address, &client_address_len);
		if (new_socket_fd == -1){
			printf("Server ERROR: accept");
			free(pthread_arg);
			continue;
		}

		/* Initialize pthread argument */
		pthread_arg->new_socket_fd = new_socket_fd;
		pthread_arg->num = 0;

		/* Create thread to serve connection to client */
		if (pthread_create(&pthread, &pthread_attr, pthread_routine, (void*)pthread_arg) != 0){
			printf("pthread_create");
			free(pthread_arg);
			continue;
		}
	}

	close(socket_fd);
	return 0;
}	
