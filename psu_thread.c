#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <bits/sigstack.h>

#include "psu_thread.h"

int g_mode = -1;
int g_port = -1;
int socket_fd = -1;
int n = -1;

static const char *msg = "client_msg";
char server_buf[16];
int i;

static const char *filename = "server_sock_info.in";
static const char *hostname_token = "hostname: ";
static const char *port_token = "port: ";
static const char *delim = ": ";

static ucontext_t uctx_foo, uctx_main;
int foo_stack[SIGSTKSZ / 4];

char remote_hostname[HOST_NAME_MAX];

static void error(const char *msg) {
	perror(msg);
	exit(0);
}

static void get_server_socket_info(void) {
		FILE *server_sock_info_fp = fopen(filename, "r");
		if (server_sock_info_fp == NULL) {
#if ERROR_LEVEL
			error("Server socket information file open failed\n");
#endif
		}
		char *line = NULL;
	    size_t len = 0;
		ssize_t read;
		int count = 0;
		char hostname[HOST_NAME_MAX];
		int port = -1;
		while ((read = getline(&line, &len, server_sock_info_fp)) != -1) {
			if (count > 1) {
#if ERROR_LEVEL
				fprintf(stderr, "Server socket information file has more lines than required\n");
#endif
				exit(0);
			}
			if (!count && strstr(line, hostname_token) == NULL) {
#if ERROR_LEVEL
				fprintf(stderr, "Server socket information file formatting incorrect, hostname: <host> not present on 1st line\n>");
#endif
				exit(0);
			}
			else if (!count) {
				char *token;
				token = strtok(line, delim);
				while (token != NULL) {
					strcpy(hostname, token);
					hostname[strcspn(hostname, "\n")] = 0;
					token = strtok(NULL, delim);
				}
#if INFO_LEVEL
				printf("Server hostname: %s\n", hostname);
#endif
				strcpy(remote_hostname, hostname);
			}
			if (count && strstr(line, port_token) == NULL) {
#if ERROR_LEVEL
				fprintf(stderr, "Server socket information file formatting incorrect, port: <port> not present on 2nd line\n>");
#endif
				exit(0);
			}
			else if (count) {
				int token_offset = 0;
				char *token;
				token = strtok(line, delim);
				while (token != NULL) {
					if (token_offset) {
						port = atoi(token);
						if (port < PORT_MIN || port > PORT_MAX) {
#if ERROR_LEVEL
							fprintf(stderr, "Port number in server socket information file not within valid range\n");
#endif
							exit(0);
						}
					}
					token = strtok(NULL, delim);
					token_offset++;
				}
#if INFO_LEVEL
				printf("Server port: %d\n", port);
#endif
				g_port = port;
			}
			count++;
		}
		fclose(server_sock_info_fp);
		if (line) {
			free(line);
		}
}

void psu_thread_setup_init(int mode) {
	// read from a file to set up the socket connection between the client and the server
	
	int sockfd, newsockfd, portno, n = -1;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	struct hostent *server;
		
	get_server_socket_info();
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
#if ERROR_LEVEL
		error("Socket open failed\n");
#endif
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(g_port);

	if (!mode) {
		server = gethostbyname(remote_hostname);
		if (server == NULL) {
#if ERROR_LEVEL
			fprintf(stderr, "No such host\n");
#endif
			exit(0);
		}
		bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
		if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
#if ERROR_LEVEL
			error("Connection failed\n");
#endif
		}
		socket_fd = sockfd;
	}
	else if (mode == 1) {
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) {
#if ERROR_LEVEL
			error("Socket open failed\n");
#endif
		}
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
#if ERROR_LEVEL
			error("Bind failed\n");
#endif
		}
		listen(sockfd,5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
#if ERROR_LEVEL
			error("Accept failed\n");
#endif
		}
		socket_fd = newsockfd;
	}
	else {
#if ERROR_LEVEL
		fprintf(stderr, "mode is incorrect\n");
#endif
		exit(0);
	}
	g_mode = mode;
	return;
}

int psu_thread_create(void * (*user_func)(void*), void *user_args) {
	// make thread related setup
	// create thread and start running the function based on *user_func
	
	if (!g_mode) {
		if (getcontext(&uctx_foo) == -1) {
#if ERROR_LEVEL
			error("Get context in main failed\n");
#endif
		}
		uctx_foo.uc_stack.ss_sp = foo_stack;
		uctx_foo.uc_stack.ss_size = sizeof(foo_stack);
		makecontext(&uctx_foo, (void (*)(void)) user_func, 1, user_args);
		if (swapcontext(&uctx_main, &uctx_foo) == -1) {
			error("Swap context from main to foo in main thread failed\n");
		}
	}
	else if (g_mode == 1) {
		n = read(socket_fd, server_buf, strlen(msg));
		if (n < 0) {
#if ERROR_LEVEL
			error("Read from socket failed\n");
#endif
		}
		else if (n < strlen(msg)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Read less number of bytes from the socket than expected\n", __LINE__);
#endif
		}
	}
	return 0; 
}

void psu_thread_migrate(const char *hostname) {
	// thread migration related code
	if (!g_mode) {
		n = write(socket_fd, msg, strlen(msg));
		if (n < 0) {
#if ERROR_LEVEL
			error("Write to socket failed\n");
#endif
		}
		else if (n < strlen(msg)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Written less number of bytes to the socket than expected\n", __LINE__);
#endif
		}
		if (getcontext(&uctx_foo) == -1) {
#if ERROR_LEVEL
			error("Get context in main failed\n");
#endif
		}
		printf("Stack data-\n");
		for (i = 0; i < SIGSTKSZ / 4; i++) {
			printf("%x\t", foo_stack[i]);
		}
		printf("Stack addresses-\n");
		for (i = 0; i < SIGSTKSZ / 4; i++) {
			printf("%x\t", &foo_stack[i]);
		}
		printf("\n");
		// printf("%u\n", sizeof(size_t));
	}

	return;
}
