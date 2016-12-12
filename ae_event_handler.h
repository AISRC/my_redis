#ifndef __AE_EVENT_HANDLER_H__
#define __AE_EVENT_HANDLER_H__

#include "ae.h"
#include "zmalloc.h"

#define REPLY_CHUNK_BYTE 1024*4		/* output buffer is 4KB */
#define QUERY_CHUNK_BYTE 1024*4		/* query buffer */
#define MAX_CLIENTS 16				/* the max number of clients */

#define REDIS_IP_STR_LEN 46 /* INET6_ADDRSTRLEN is 46, but we need to be sure */

typedef struct redisClient {
	int client_fd;
	char query_buf[QUERY_CHUNK_BYTE];
	int output_len;
	char output_buf[REPLY_CHUNK_BYTE];
}redisClient;

typedef struct redisServer {
	pid_t pid;					/* process id */
	aeEventLoop *event_loop;
	int port;					/* TCP listening port */
	int server_fd;
	int tcp_backlog;			/* TCP listen() backlog */
	int current_client_num;
	redisClient* clients[MAX_CLIENTS];
}redisServer;

extern redisServer g_server;

/* handlers */
void acceptTcpHandler (aeEventLoop *el, int fd, void *privdata, int mask);
void recvFromClientHandler (aeEventLoop *el, int fd, void *privdata, int mask);
void replyToClientHandler (aeEventLoop *el, int fd, void *privdata, int mask);

/* concrete handle implementation */
/* Networking and Client related operations */
redisClient* createClient (int fd, aeEventLoop *el);
void freeClient (redisClient *c);
void sendReplyToClient (aeEventLoop *event_loop, int fd, void *privdata, int mask);
void readQueryFromClient (aeEventLoop *event_loop, int fd, void *privdata, int mask);
void processBuffer (aeEventLoop *event_loop, redisClient *c);

#endif
