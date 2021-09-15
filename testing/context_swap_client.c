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
int n = -1;

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
	if (!mode) {
		// send ucontext uctx_foo to server
		/* send sigmask
		 */
		n = write(socket_fd, &uctx_foo.uc_sigmask, sizeof(sigset_t));
		if (n < 0) {
#if ERROR_LEVEL
			error("Write to socket failed\n");
#endif
		}
		else if (n < sizeof(sigset_t)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Written less number of bytes to the socket than expected\n", __LINE__);
#endif
		}
		char recv_code[16];
		/* receive sigmask code
		 */
		n = read(socket_fd, recv_code, strlen(sigmask_recv));
		if (n < 0) {
#if ERROR_LEVEL
			error("Read from socket failed\n");
#endif
		}
		else if (n < strlen(sigmask_recv)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Read less number of bytes from the socket than expected\n", __LINE__);
#endif
		}
		recv_code[strlen(sigmask_recv)] = '\0';
		if (strcmp(recv_code, sigmask_recv)) {
#if ERROR_LEVEL
			fprintf(stderr, "Sigmask recv code obtained from server is incorrect\n");
#endif
			exit(0);
		}
		else {
#if DEBUG_LEVEL
			printf("Sigmask recv code correctly obtained from server\n");
#endif
		}
		/* send stack base
		 */
		unsigned char *ss_sp_data = malloc(sizeof(void *));
		if (ss_sp_data == NULL) {
#if ERROR_LEVEL
			error("Malloc failed for stack base data\n");
#endif
		}
		memcpy(ss_sp_data, &uctx_foo.uc_stack.ss_sp, sizeof(void *));
#if DEBUG_LEVEL
		printf("Stack base = %p\n", uctx_foo.uc_stack.ss_sp);
#endif
		n = write(socket_fd, ss_sp_data, sizeof(void *));
		if (n < 0) {
#if ERROR_LEVEL
			error("Write to socket failed\n");
#endif
		}
		else if (n < sizeof(size_t)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Written less number of bytes to the socket than expected\n", __LINE__);
#endif
		}
		/* receive stack base code
		 */
		n = read(socket_fd, recv_code, strlen(stack_base_recv));
		if (n < 0) {
#if ERROR_LEVEL
			error("Read from socket failed\n");
#endif
		}
		else if (n < strlen(stack_base_recv)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Read less number of bytes from the socket than expected\n", __LINE__);
#endif
		}
		recv_code[strlen(stack_base_recv)] = '\0';
		if (strcmp(recv_code, stack_base_recv)) {
#if ERROR_LEVEL
			fprintf(stderr, "Stack base recv code obtained from server is incorrect\n");
#endif
			exit(0);
		}
		else {
#if DEBUG_LEVEL
			printf("Stack base recv code correctly obtained from server\n");
#endif
		}
		/* send stack size
		 */
		n = write(socket_fd, &uctx_foo.uc_stack.ss_size, sizeof(size_t));
		if (n < 0) {
#if ERROR_LEVEL
			error("Write to socket failed\n");
#endif
		}
		else if (n < sizeof(size_t)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Written less number of bytes to the socket than expected\n", __LINE__);
#endif
		}
		/* receive stack size code
		 */
		n = read(socket_fd, recv_code, strlen(stack_size_recv));
		if (n < 0) {
#if ERROR_LEVEL
			error("Read from socket failed\n");
#endif
		}
		else if (n < strlen(stack_size_recv)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Read less number of bytes from the socket than expected\n", __LINE__);
#endif
		}
		recv_code[strlen(stack_size_recv)] = '\0';
		if (strcmp(recv_code, stack_size_recv)) {
#if ERROR_LEVEL
			fprintf(stderr, "Stack size recv code obtained from server is incorrect\n");
#endif
			exit(0);
		}
		else {
#if DEBUG_LEVEL
			printf("Stack size recv code correctly obtained from server\n");
#endif
		}
		/* send stack flags
		 */
		n = write(socket_fd, &uctx_foo.uc_stack.ss_flags, sizeof(int));
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
		/* receive stack flags code
		 */
		n = read(socket_fd, recv_code, strlen(stack_flags_recv));
		if (n < 0) {
#if ERROR_LEVEL
			error("Read from socket failed\n");
#endif
		}
		else if (n < strlen(stack_flags_recv)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Read less number of bytes from the socket than expected\n", __LINE__);
#endif
		}
		recv_code[strlen(stack_flags_recv)] = '\0';
		if (strcmp(recv_code, stack_flags_recv)) {
#if ERROR_LEVEL
			fprintf(stderr, "Stack flags recv code obtained from server is incorrect\n");
#endif
			exit(0);
		}
		else {
#if DEBUG_LEVEL
			printf("Stack flags recv code correctly obtained from server\n");
#endif
		}
		/* send mcontext
		 */
		n = write(socket_fd, &uctx_foo.uc_mcontext, sizeof(mcontext_t));
		if (n < 0) {
#if ERROR_LEVEL
			error("Write to socket failed\n");
#endif
		}
		else if (n < sizeof(mcontext_t)) {
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
		/* receive mcontext code
		 */
		n = read(socket_fd, recv_code, strlen(mcontext_recv));
		if (n < 0) {
#if ERROR_LEVEL
			error("Read from socket failed\n");
#endif
		}
		else if (n < strlen(mcontext_recv)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Read less number of bytes from the socket than expected\n", __LINE__);
#endif
		}
		recv_code[strlen(mcontext_recv)] = '\0';
		if (strcmp(recv_code, mcontext_recv)) {
#if ERROR_LEVEL
			// printf("Mcontext recv code = %s\n", recv_code);
			fprintf(stderr, "Mcontext recv code obtained from server is incorrect\n");
#endif
			exit(0);
		}
		else {
#if DEBUG_LEVEL
			printf("Mcontext recv code correctly obtained from server\n");
#endif
		}
		/* send stack
		 */
		n = write(socket_fd, uctx_foo.uc_stack.ss_sp, uctx_foo.uc_stack.ss_size);
		if (n < 0) {
#if ERROR_LEVEL
			error("Write to socket failed\n");
#endif
		}
		else if (n < uctx_foo.uc_stack.ss_size) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Written less number of bytes to the socket than expected\n", __LINE__);
#endif
		}
		/* receive stack code
		 */
		n = read(socket_fd, recv_code, strlen(stack_recv));
		if (n < 0) {
#if ERROR_LEVEL
			error("Read from socket failed\n");
#endif
		}
		else if (n < strlen(stack_recv)) {
#if WARN_LEVEL
			fprintf(stdout, "%d: Read less number of bytes from the socket than expected\n", __LINE__);
#endif
		}
		recv_code[strlen(stack_recv)] = '\0';
		if (strcmp(recv_code, stack_recv)) {
#if ERROR_LEVEL
			fprintf(stderr, "Stack recv code obtained from server is incorrect\n");
#endif
			exit(0);
		}
		else {
#if DEBUG_LEVEL
			printf("Stack recv code correctly obtained from server\n");
#endif
		}
		if (!mode) {
			// if (getcontext(&uctx_foo) == -1) {
#if ERROR_LEVEL
			//	error("Get context in main failed\n");
#endif
			// }
#if DEBUG_LEVEL
			printf("Latter stack\n");
			for (int i = 0; i < uctx_foo.uc_stack.ss_size; i += 4) {
				printf("%d\t", *(((int *) uctx_foo.uc_stack.ss_sp) + (i / 4)));
			}
			printf("\n");
#endif
			// pthread_exit(NULL);
			if (swapcontext(&uctx_foo, &uctx_main) == -1) {
				error("Swap context from foo to main in foo thread imitation failed\n");
			}
		}
	}
	/* psu_thread_migrate END
	 */
	print_hostname();
	printf("Foo: Exit\n");
	return NULL;
}

int main(int argc, char *argv[]) {
	// if (getcontext(&uctx_foo) == -1) {
#if ERROR_LEVEL
	// 	error("Get context in main failed\n");
#endif
	// }
	// Test code
	// char foo_stack[SIGSTKSZ];
	// uctx_foo.uc_stack.ss_sp = foo_stack;
	// uctx_foo.uc_stack.ss_size = sizeof(foo_stack);
	// uctx_foo.uc_link = &uctx_main;
	// makecontext(&uctx_foo, (void (*)(void)) foo, 1, NULL);
	// if (getcontext(&uctx_foo) == -1) {
#if ERROR_LEVEL
	// 	error("Get context in main failed\n");
#endif
	// }
	// printf("stack base = %p\n", uctx_foo.uc_stack.ss_sp);
	// void *ptr = NULL;
	// unsigned char *ss_sp_data = malloc(sizeof(void *));
	// memcpy(ss_sp_data, &uctx_foo.uc_stack.ss_sp, sizeof(void *));
	// memcpy(&ptr, ss_sp_data, sizeof(void *));
	// printf("stack base ptr = %p\n", ptr);
	// printf("stack size = %lu\n", uctx_foo.uc_stack.ss_size);
	// exit(0);

	int sockfd, portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	if (argc < 3) {
#if ERROR_LEVEL
		fprintf(stderr, "usage %s host mode\n", argv[0]);
#endif
		exit(0);
	}

	/* psu_thread_setup_init BEGIN 
	 */
	if (!atoi(argv[2])) {
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
		server = gethostbyname(hostname);
		if (server == NULL) {
#if ERROR_LEVEL
			fprintf(stderr, "No such host\n");
#endif
			exit(0);
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
		serv_addr.sin_port = htons(port);
		if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
#if ERROR_LEVEL
			error("Connection failed\n");
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
	socket_fd = sockfd;
	/* psu_thread_setup_init END
	 */

	/* psu_thread_create BEGIN
	 */
	if (!mode) {
		char foo_stack[SIGSTKSZ];
		pthread_t thread;

		if (getcontext(&uctx_foo) == -1) {
#if ERROR_LEVEL
			error("Get context in main failed\n");
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
		makecontext(&uctx_foo, (void (*)(void)) foo, 1, NULL);

#if DEBUG_LEVEL
		printf("Initial stack\n");
		for (int i = 0; i < uctx_foo.uc_stack.ss_size; i += 4) {
			printf("%d\t", *(((int *) uctx_foo.uc_stack.ss_sp) + (i / 4)));
		}
		printf("\n");
#endif

		// pthread_create(&thread, NULL, foo, NULL);

		// pthread_join(thread, NULL);
		
		if (swapcontext(&uctx_main, &uctx_foo) == -1) {
			error("Swap context from main to foo in main thread failed\n");
		}
	}
	/* psu_thread_create END
	 */

	return 0;
}
