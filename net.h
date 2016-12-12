#ifndef __NET_H__
#define __NET_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "zmalloc.h"

#define CLI_QUERY_CHUNK_BYTE 1024*4

#define CLI_ERR -1
#define CLI_OK 0

typedef struct redisContext {
	int fd; 
	char buf[CLI_QUERY_CHUNK_BYTE];
}redisContext;

redisContext *redisConnect (const char *ip, int port);
int redisContextConnectTcp (redisContext *c, const char *addr, int port, 
		const struct timeval *timeout);

void redisFreeContext (redisContext *c);
#endif
