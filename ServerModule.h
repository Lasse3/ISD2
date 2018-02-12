#ifndef SERVERMODULE_H_
#define SERVERMODULE_H_

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define THREADS 5

extern FILE *fp;

typedef struct
{
	FILE *fp;
	int sockfd;
} serverObject;

void *server_handler(void *arg);
void *client_handler(void *arg);

#endif
