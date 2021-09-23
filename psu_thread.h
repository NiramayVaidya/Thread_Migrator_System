#ifndef PSU_THREAD_H
#define PSU_THREAD_H

#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ucontext.h>
#include <pthread.h>
#include <limits.h>
#include <signal.h>
#include <bits/sigstack.h>

#ifdef __USE_GNU
#define EBP REG_EBP
#define ESP REG_ESP
#define EIP REG_EIP
#define UESP REG_UESP
#else
#define EBP 6
#define ESP 7
#define EIP 14
#define UESP 17
#endif

#define DEBUG_LEVEL 1
#define INFO_LEVEL 1
#define WARN_LEVEL 1
#define ERROR_LEVEL 1

#define PORT_MIN 1024
#define PORT_MAX 65535

typedef struct psu_thread_info {
	int mode;
	char hostname[HOST_NAME_MAX];
	int port;
	int sock_fd;
	ucontext_t uctx_user_func;
	int user_func_stack[SIGSTKSZ / 4];
	void (*user_func)(void);
} psu_thread_info_t;

void psu_thread_setup_init(int mode);

int psu_thread_create(void *(*user_func)(void *), void *user_args);

void psu_thread_migrate(const char *hostname);

#endif /* PSU_THREAD_H  */
