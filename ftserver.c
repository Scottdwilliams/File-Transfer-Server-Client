#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <curses.h>

#define LISTEN_ENQ 5
#define SRV_PORT 5103
#define MAX_RECV_BUF 1452
#define MAX_SEND_BUF 1452

void get_file_name(int, char*);
int send_file(int, char*);
void sig_chld(int);

int main(int argc, char* argv[]){

	int listen_fd, conn_fd;
	struct sockaddr_in srv_addr, cli_addr;
	socklen_t cli_len;
	pid_t child_pid;

	char file_name [MAX_RECV_BUF]; /* name of the flie being sent */
	char print_addr [INET_ADDRSTRLEN]; /* readable IP Address */

	memset(&srv_addr, 0, sizeof(srv_addr));
	memset(&cli_addr, 0, sizeof(cli_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	// if port number supplied, use it, otherwise use SRV_PORT
	srv_addr.sin_port = (argc > 1) ? htons(atoi(argv[1])) : htons(SRV_PORT);

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	//bind to created socket
	if (bind(listen_fd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0){
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	printf("Listening on port number %d ...\n", ntohs(srv_addr.sin_port));
	if (listen(listen_fd, LISTEN_ENQ) < 0){
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	signal(SIGCHLD, sig_chld);

	while (1){ //run forever
		cli_len = sizeof(cli_addr);
		printf("Waiting for a client to connect...\n\n");
		//block until a client connects
		if ((conn_fd = accept(listen_fd, (struct sockaddr*) &cli_addr, &cli_len)) < 0){
			perror("accept error");
			break;
		}

		inet_ntop(AF_INET, &(cli_addr.sin_addr), print_addr, INET_ADDRSTRLEN);
		printf("Client connected from %s:%d\n", print_addr, ntohs(cli_addr.sin_port));

		if ((child_pid = fork()) == 0) {
			close(listen_fd);

			get_file_name(conn_fd, file_name);
			send_file(conn_fd, file_name);
			printf("Closing connection\n");
			close(conn_fd);
			exit(0);
		}

		close(conn_fd);	//close parent's copy of conn_fd
		close(listen_fd);
		return 0;
	}
}


void get_file_name(int sock, char* file_name){
	char recv_str[MAX_RECV_BUF];
	ssize_t rcvd_bytes;

	//red name of requested file
	if ((rcvd_bytes = recv(sock, recv_str, MAX_RECV_BUF, 0)) < 0) {
		perror("recv error");
		return;
	}

	sscanf(recv_str, "%s\n", file_name);
}


int send_file(int sock, char *file_name) {
	int sent_count;	//how many chunks sent
	ssize_t read_bytes, sent_bytes, sent_file_size;
	char send_buf[MAX_SEND_BUF];
	char * errmsg_notfound = "File not found\n";
	int f; //file

	sent_count = 0;
	sent_file_size = 0;

	//open requested file
	if((f = open(file_name, O_RDONLY)) < 0) {
		perror(file_name);
		if((sent_bytes = send(sock, errmsg_notfound, strlen(errmsg_notfound), 0)) < 0){
			perror("send error");
			return -1;
		}
	}

	else {	//open is sucessful, send file
		printf("Sending file: %s\n", file_name);
		while ((read_bytes = read(f, send_buf, MAX_RECV_BUF)) > 0){
			if((sent_bytes = send(sock, send_buf, read_bytes, 0)) < read_bytes) {
				perror("send error");
				return -1;
			}
		
			sent_count++;
			sent_file_size += sent_bytes;
		}
		close(f);
	}
	printf("Sent %d bytes in %d chunk(s)\n\n", sent_file_size, sent_count);
	return sent_count;
}

void sig_chld(int signo){
	pid_t pid;
	int stat;

	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0){
		printf("Child %d terminated\n", pid);
	}
	return;
}
