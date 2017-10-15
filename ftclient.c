#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define SRV_PORT 5103
#define MAX_RECV_BUF 1452
#define MAX_SEND_BUF 1452

int recv_file(int, char*);

int main(int argc, char* argv[]){

	int sock_fd;
	struct sockaddr_in srv_addr;

	if (argc < 3) {
		printf("Usage: %s <filename> <IP address> [port number]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	memset(&srv_addr, 0, sizeof(srv_addr));

	//create slient socket
	sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	srv_addr.sin_family = AF_INET;

	if (inet_pton(AF_INET, argv[2], &(srv_addr.sin_addr)) < 1){
		printf("Invalid IP address\n");
		exit(EXIT_FAILURE);
	}

	//if port is supplied, use it, otherwise use SRV_PORT
	srv_addr.sin_port = (argc > 3) ? htons(atoi(argv[3])) : htons(SRV_PORT);

	if (connect(sock_fd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0){
		perror("connection error");
		exit(EXIT_FAILURE);
	}

	printf("Connected to: %s:%d ..\n", argv[2],SRV_PORT);

	recv_file(sock_fd, argv[1]);
	if (close(sock_fd) < 0){
		perror("socket close error");
		exit(EXIT_FAILURE);
	}
	return 0;
}


int recv_file(int sock, char* file_name) {

	char send_str [MAX_SEND_BUF];
	int f;	//file
	ssize_t sent_bytes, rcvd_bytes, rcvd_file_size;
	int recv_count;
	char recv_str[MAX_RECV_BUF];
	size_t send_strlen;

	sprintf(send_str, "%s\n", file_name);
	send_strlen = strlen(send_str);

	if ((sent_bytes = send(sock, file_name, send_strlen, 0)) < 0) {
		perror("send error");
		return -1;
	}

	if ((f = open(file_name, O_WRONLY|O_CREAT, 0644)) < 0 ){
		perror("error creating file");
		return -1;
	}

	recv_count = 0;
	rcvd_file_size = 0;

	while ((rcvd_bytes = recv(sock, recv_str, MAX_RECV_BUF, 0)) > 0){
		recv_count++;
		rcvd_file_size += rcvd_bytes;

		if (write(f, recv_str, rcvd_bytes) < 0){
			perror("error writing to file");
			return -1;
		}
	}

	close(f);
	printf("Client received: %d bytes in %d chunk(s)\n", rcvd_file_size, recv_count);
	return rcvd_file_size;
}
