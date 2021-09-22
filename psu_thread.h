#ifndef PSU_THREAD_H
#define PSU_THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ucontext.h>
#include <pthread.h>

#define DEBUG_LEVEL 1
#define INFO_LEVEL 1
#define WARN_LEVEL 1
#define ERROR_LEVEL 1

#define PORT_MIN 1024
#define PORT_MAX 65535

// typedef struct psu_thread_info psu_thread_info_t;

void psu_thread_setup_init(int mode);

int psu_thread_create(void * (*user_func)(void*), void *user_args);

void psu_thread_migrate(const char *hostname);

#endif /* PSU_THREAD_H  */
