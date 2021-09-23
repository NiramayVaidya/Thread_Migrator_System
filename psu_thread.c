#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "psu_thread.h"

static const char *filename = "server_sock_info.in";
static const char *hostname_token = "hostname: ";
static const char *port_token = "port: ";
static const char *delim = ": ";

int n = -1, i = 0, user_func_offset = 0;

static ucontext_t uctx_curr;

psu_thread_info_t thread_info;

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
				strcpy(thread_info.hostname, hostname);
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
				thread_info.port = port;
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
	
	int sockfd, newsockfd, portno;
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
	serv_addr.sin_port = htons(thread_info.port);

	if (!mode) {
		server = gethostbyname(thread_info.hostname);
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
		thread_info.sock_fd = sockfd;
	}
	else if (mode == 1) {
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
#if ERROR_LEVEL
			error("Bind failed\n");
#endif
		}
		listen(sockfd, 5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
#if ERROR_LEVEL
			error("Accept failed\n");
#endif
		}
		thread_info.sock_fd = newsockfd;
	}
	else {
#if ERROR_LEVEL
		fprintf(stderr, "mode is incorrect\n");
#endif
		exit(0);
	}
	thread_info.mode = mode;
	return;
}

int psu_thread_create(void *(*user_func)(void *), void *user_args) {
	// make thread related setup
	// create thread and start running the function based on *user_func
	
	thread_info.user_func = (void (*)(void)) user_func;
#if DEBUG_LEVEL
	printf("thread info user_func- %x\n", (unsigned int) thread_info.user_func);
	printf("user_func- %x\n", (unsigned int) user_func);
#endif
	if (getcontext(&thread_info.uctx_user_func) == -1) {
#if ERROR_LEVEL
		error("Get context in psu thread create failed\n");
#endif
	}
	thread_info.uctx_user_func.uc_stack.ss_sp = thread_info.user_func_stack;
	thread_info.uctx_user_func.uc_stack.ss_size = sizeof(thread_info.user_func_stack);
	if (!thread_info.mode) {
		makecontext(&thread_info.uctx_user_func, thread_info.user_func, 1, user_args);
		if (swapcontext(&uctx_curr, &thread_info.uctx_user_func) == -1) {
			error("Swap context from current context to user_func in psu thread create failed\n");
		}
	}
	else if (thread_info.mode == 1) {
		n = read(thread_info.sock_fd, &user_func_offset, sizeof(int));
		if (n < 0) {
#if ERROR_LEVEL
			error("Read from socket failed\n");
#endif
		}
		else if (n < sizeof(int)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Read less number of bytes from the socket than expected\n", __LINE__);
#endif
		}
		if (getcontext(&thread_info.uctx_user_func) == -1) {
#if ERROR_LEVEL
			error("Get context in psu thread create failed\n");
#endif
		}
#if DEBUG_LEVEL
		/*
		printf("Stack data-\n");
		for (i = 0; i < SIGSTKSZ / 4; i++) {
			printf("%d : %x\t", i, thread_info.user_func_stack[i]);
		}
		printf("Stack addresses-\n");
		for (i = 0; i < SIGSTKSZ / 4; i++) {
			printf("%d : %x\t", &thread_info.user_func_stack[i]);
		}
		printf("\n");
		*/
#endif
#if DEBUG_LEVEL
		printf("user func offset- %d\n", user_func_offset);
		printf("user func- %x\n", (unsigned int) user_func);
		printf("user func + user func offset- %x\n", (unsigned int) user_func + user_func_offset);
#endif
		// makecontext(&thread_info.uctx_user_func, (void (*)(void)) user_func + user_func_offset, 1, user_args);
		makecontext(&thread_info.uctx_user_func, (void (*)(void)) user_func, 1, user_args);
		thread_info.uctx_user_func.uc_mcontext.gregs[EIP] = user_func + user_func_offset;
		thread_info.uctx_user_func.uc_mcontext.gregs[EBP] = (int *) thread_info.uctx_user_func.uc_stack.ss_sp + (thread_info.uctx_user_func.uc_stack.ss_size / 4) - 1;
#if DEBUG_LEVEL
		printf("User func stack data-\n");
		for (i = 0; i < SIGSTKSZ / 4; i++) {
			printf("%d : %x\t", i, thread_info.user_func_stack[i]);
		}
		printf("User func stack addresses-\n");
		for (i = 0; i < SIGSTKSZ / 4; i++) {
			printf("%d : %x\t", i, (unsigned int) &thread_info.user_func_stack[i]);
		}
		printf("\n");
#endif
		if (swapcontext(&uctx_curr, &thread_info.uctx_user_func) == -1) {
			error("Swap context from current context to user func in psu thread create failed\n");
		}
	}
	return 0; 
}

void psu_thread_migrate(const char *hostname) {
	// thread migration related code

	if (!thread_info.mode) {
		if (getcontext(&thread_info.uctx_user_func) == -1) {
#if ERROR_LEVEL
			error("Get context in psu thread migrate failed\n");
#endif
		}
#if DEBUG_LEVEL
		/*
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
		*/
#endif
#if DEBUG_LEVEL
		printf("ebp- %x\n", thread_info.uctx_user_func.uc_mcontext.gregs[EBP]);
		printf("ss_sp- %x\n", thread_info.uctx_user_func.uc_stack.ss_sp);

		int ebp_offset = thread_info.uctx_user_func.uc_mcontext.gregs[EBP] - (unsigned int) thread_info.uctx_user_func.uc_stack.ss_sp;
		printf("ebp offset- %d\n", ebp_offset);

		int eip_offset = ebp_offset + 4;
		int eip_stack_index = ebp_offset / 4 + 1;
		printf("eip offset- %d\n", eip_offset);
		printf("eip stack index- %d\n", eip_stack_index);
		int eip_value = ((int *) thread_info.uctx_user_func.uc_stack.ss_sp)[eip_stack_index];
		printf("user func- %x\n", (unsigned int) thread_info.user_func);
		printf("eip value- %x\n", eip_value);
		user_func_offset = eip_value - (unsigned int) thread_info.user_func;
		printf("user func offset- %d\n", user_func_offset);
#endif
		n = write(thread_info.sock_fd, &user_func_offset, sizeof(int));
		if (n < 0) {
#if ERROR_LEVEL
			error("Write to socket failed\n");
#endif
		}
		else if (n < sizeof(int)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Written less number of bytes to the socket than expected\n", __LINE__);
#endif
		}
		exit(0);
	}

	return;
}
