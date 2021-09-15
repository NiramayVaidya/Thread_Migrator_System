#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <ucontext.h>
#include <bits/sigstack.h>
#include <signal.h>

#define DEBUG_LEVEL 1
#define INFO_LEVEL 1
#define WARN_LEVEL 1
#define ERROR_LEVEL 1

#define PORT_MIN 1024
#define PORT_MAX 65535

int mode = -1;
int socket_fd = -1;

static const char *sigmask_recv = "SIGMSK_RECV";
static const char *stack_base_recv = "STK_BASE_RECV";
static const char *stack_size_recv = "STK_SIZE_RECV";
static const char *stack_flags_recv = "STK_FLAGS_RECV";
static const char *mcontext_recv = "MCONTEXT_RECV";
static const char *stack_recv = "STACK_RECV";

static const char *filename = "server_sock_info.in";
static const char *hostname_token = "hostname: ";
static const char *port_token = "port: ";
static const char *delim = ": ";

static ucontext_t uctx_foo, uctx_main;

char REMOTE_HOSTNAME[255];
char remote_hostname[HOST_NAME_MAX];

void error(const char *msg) {
	perror(msg);
	exit(0);
}

void print_hostname() {
	char buffer[100];
	int ret;
	if ((ret = gethostname(buffer, sizeof(buffer))) == -1) {
		error("Gethostname failed\n");
	}
	printf("Hostname: %s\n", buffer);
}

void *foo(void *arg) {
	print_hostname();
	printf("Foo: Entry\n");
	/* psu_thread_migrate BEGIN
	 */
	// if (strcmp(remote_hostname, REMOTE_HOSTNAME)) {
	// 	error("Remote hostname passed to thread migrate does not match with hostname provided in the server socket information file\n");
	// }
	// receive ucontext uctx_foo from client?
	if (!mode) {
		// send ucontext uctx_foo to server
		if (!mode) {
			pthread_exit(NULL);
		}
	}
	/* psu_thread_migrate END
	 */
	print_hostname();
	printf("Foo: Exit\n");
	return NULL;
}

int main(int argc, char *argv[]) {
	int sockfd, newsockfd, portno, n = -1;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	if (argc < 3) {
#if ERROR_LEVEL
		fprintf(stderr, "usage %s host mode\n", argv[0]);
#endif
		exit(0);
	}

	/* psu_thread_setup_init BEGIN 
	 */
	if (atoi(argv[2])) {
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
				strcpy(REMOTE_HOSTNAME, hostname);
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
			}
			count++;
		}
		fclose(server_sock_info_fp);
		if (line) {
			free(line);
		}
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) {
#if ERROR_LEVEL
			error("Socket open failed\n");
#endif
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(port);
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
	}
	else {
#if ERROR_LEVEL
		fprintf(stderr, "mode is incorrect\n");
#endif
		exit(0);
	}
	mode = atoi(argv[2]);
	socket_fd = newsockfd;
	/* psu_thread_setup_init END
	 */

	/* psu_thread_create BEGIN
	 */
	if (mode) {
		if (getcontext(&uctx_foo) == -1) {
#if ERROR_LEVEL
			error("Get context in main failed\n");
#endif
		}
		// receive ucontext uctx_foo from client
		/* receive sigmask
		 */
		n = read(socket_fd, &uctx_foo.uc_sigmask, sizeof(sigset_t));
		if (n < 0) {
#if ERROR_LEVEL
			error("Read from socket failed\n");
#endif
		}
		else if (n < sizeof(sigset_t)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Read less number of bytes from the socket than expected\n", __LINE__);
#endif
		}
		/* send sigmask code
		 */
		n = write(socket_fd, sigmask_recv, strlen(sigmask_recv));
		if (n < 0) {
#if ERROR_LEVEL
			error("Write to socket failed\n");
#endif
		}
		else if (n < strlen(sigmask_recv)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Written less number of bytes to the socket than expected\n", __LINE__);
#endif
		}
		/* receive stack base
		 */
		unsigned char *ss_sp_data = malloc(sizeof(void *));
		if (ss_sp_data == NULL) {
#if ERROR_LEVEL
			error("Malloc failed for stack base data\n");
#endif
		}
		n = read(socket_fd, ss_sp_data, sizeof(void *));
		if (n < 0) {
#if ERROR_LEVEL
			error("Read from socket failed\n");
#endif
		}
		else if (n < sizeof(void *)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Read less number of bytes from the socket than expected\n", __LINE__);
#endif
		}
		// memcpy(&uctx_foo.uc_stack.ss_sp, ss_sp_data, sizeof(void *));
#if DEBUG_LEVEL
		printf("Stack base = %p\n", uctx_foo.uc_stack.ss_sp);
#endif
		/* send stack base code
		 */
		n = write(socket_fd, stack_base_recv, strlen(stack_base_recv));
		if (n < 0) {
#if ERROR_LEVEL
			error("Write to socket failed\n");
#endif
		}
		else if (n < strlen(stack_base_recv)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Written less number of bytes to the socket than expected\n", __LINE__);
#endif
		}
		/* receive stack size
		 */
		n = read(socket_fd, &uctx_foo.uc_stack.ss_size, sizeof(size_t));
		if (n < 0) {
#if ERROR_LEVEL
			error("Read from socket failed\n");
#endif
		}
		else if (n < sizeof(size_t)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Read less number of bytes from the socket than expected\n", __LINE__);
#endif
		}
		/* send stack size code
		 */
		n = write(socket_fd, stack_size_recv, strlen(stack_size_recv));
		if (n < 0) {
#if ERROR_LEVEL
			error("Write to socket failed\n");
#endif
		}
		else if (n < strlen(stack_size_recv)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Written less number of bytes to the socket than expected\n", __LINE__);
#endif
		}
		/* receive stack flags
		 */
		n = read(socket_fd, &uctx_foo.uc_stack.ss_flags, sizeof(int));
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
		/* send stack flags code
		 */
		n = write(socket_fd, stack_flags_recv, strlen(stack_flags_recv));
		if (n < 0) {
#if ERROR_LEVEL
			error("Write to socket failed\n");
#endif
		}
		else if (n < strlen(stack_flags_recv)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Written less number of bytes to the socket than expected\n", __LINE__);
#endif
		}
		/* receive mcontext
		 */
		n = read(socket_fd, &uctx_foo.uc_mcontext, sizeof(mcontext_t));
		if (n < 0) {
#if ERROR_LEVEL
			error("Read from socket failed\n");
#endif
		}
		else if (n < sizeof(mcontext_t)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Read less number of bytes from the socket than expected\n", __LINE__);
#endif
		}
		/* send mcontext code
		 */
		n = write(socket_fd, mcontext_recv, strlen(mcontext_recv));
		if (n < 0) {
#if ERROR_LEVEL
			error("Write to socket failed\n");
#endif
		}
		else if (n < strlen(mcontext_recv)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Written less number of bytes to the socket than expected\n", __LINE__);
#endif
		}
#if DEBUG_LEVEL
		int *ptr = (int *) &uctx_foo.uc_mcontext;
		for (int i = 0; i < sizeof(mcontext_t) / 4; i++) {
			printf("%d\t", *ptr++);
		}
		printf("\n");
#endif
		// if (getcontext(&uctx_foo) == -1) {
#if ERROR_LEVEL
		// 	error("Get context in main failed\n");
#endif
		// }
		char foo_stack[SIGSTKSZ];
		/* receive stack
		 */
		n = read(socket_fd, foo_stack, SIGSTKSZ);
		if (n < 0) {
#if ERROR_LEVEL
			error("Read from socket failed\n");
#endif
		}
		else if (n < SIGSTKSZ) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Read less number of bytes from the socket than expected\n", __LINE__);
#endif
		}
		/* send stack code
		 */
		n = write(socket_fd, stack_recv, strlen(stack_recv));
		if (n < 0) {
#if ERROR_LEVEL
			error("Write to socket failed\n");
#endif
		}
		else if (n < strlen(stack_recv)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Written less number of bytes to the socket than expected\n", __LINE__);
#endif
		}
#if DEBUG_LEVEL
		printf("Former stage stack\n");
		for (int i = 0; i < sizeof(foo_stack); i += 4) {
			printf("%d\t", foo_stack[i]);
		}
		printf("\n");
#endif
		uctx_foo.uc_stack.ss_sp = foo_stack;
		uctx_foo.uc_stack.ss_size = sizeof(foo_stack);
		// uctx_foo.uc_link = &uctx_main;
#if DEBUG_LEVEL
		// ptr = (int *) &uctx_foo.uc_mcontext;
		// for (int i = 0; i < sizeof(mcontext_t) / 4; i++) {
		// 	printf("%d\t", *ptr++);
		// }
		// printf("\n");
#endif
#if DEBUG_LEVEL
			printf("Initial stack\n");
			for (int i = 0; i < uctx_foo.uc_stack.ss_size; i += 4) {
				printf("%d\t", *(((int *) uctx_foo.uc_stack.ss_sp) + (i / 4)));
			}
			printf("\n");
#endif
		makecontext(&uctx_foo, (void (*)(void)) foo, 1, NULL);
		if (swapcontext(&uctx_main, &uctx_foo) == -1) {
			error("Swap context from main to foo in main thread failed\n");
		}
	}
	/* psu_thread_create END
	 */

	return 0;
}
