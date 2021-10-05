// Sample Application 7

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "psu_thread.h"

char REMOTE_HOSTNAME[255];

void print_hostname() {
	char buffer[100];
	int ret;
	if ((ret = gethostname(buffer, sizeof(buffer))) == -1) {
		perror("gethostname");
		exit(1);
	}
	printf("Hostname: %s\n", buffer);
}

void *foo(void *arg) {
	int i, j;
	int n = (int *) arg;
	print_hostname();
	printf("Foo: Entry\n");
	for (i = 0; i < n; i++) {
		printf("%d\t", i);
	}
	printf("\n");
	psu_thread_migrate(REMOTE_HOSTNAME);
	print_hostname();
	for (j = i - 1; j >= 0; j--) {
		printf("%d\t", j);
	}
	printf("\n");
	printf("Foo: Exit\n");
	return NULL;
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Waiting for goodbye\n");
		return 0;
	}

	psu_thread_setup_init(atoi(argv[2]));

	strcpy(REMOTE_HOSTNAME, argv[1]);
	psu_thread_create(foo, (void *) 10);

	printf("Main: Exit\n");

	return 0;
}
