#ifndef __ANET_H__
#define __ANET_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define ANET_OK 0
#define ANET_ERR -1

int anetTcpAccept (int serversock, char *ip, size_t ip_len, int *port);
int anetTcpServer (int port, char *bindaddr, int backlog);

int anetNonBlock (int fd);
int anetSetBlock (int fd, int non_block);

// static int anetGenericAccept (int s, struct sockaddr *sa, socklen_t *len);
#endif