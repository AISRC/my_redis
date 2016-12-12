#include "anet.h"
#include "ae_event_handler.h"

#include <string.h>
#include <errno.h>

/*********************************************************************************
 *********************************************************************************
 * handlers
 *********************************************************************************
 */

redisServer g_server;

static void acceptCommonHandler (int client_fd, aeEventLoop *el) {
	redisClient *c;
	if ((c = createClient (client_fd, el)) == NULL) {
		fprintf (stderr, "Error registering fd event for new client : %s (fd = %d), %d\n", 
		strerror (errno), client_fd, __LINE__);
		close (client_fd);
		return;
	}
}
 
void acceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
    int cport, cfd, max = MAX_CLIENTS;
    char cip[REDIS_IP_STR_LEN];
    REDIS_NOTUSED(el);
    REDIS_NOTUSED(mask);
    REDIS_NOTUSED(privdata);

    while(max--) {
        cfd = anetTcpAccept(fd, cip, sizeof(cip), &cport);
        if (cfd == ANET_ERR) {
            if (errno != EWOULDBLOCK)
                fprintf (stderr, "Accept client connection failed, %s : %d\n", __FILE__, __LINE__);
            return;
        }
        fprintf (stdout, "Accepted client(fd=%d) %s:%d\n", cfd, cip, cport);
		fflush (stdout);	/* refresh standard output buffer in user space */

        acceptCommonHandler (cfd, el);
    }
}

void recvFromClientHandler (aeEventLoop *el, int fd, void *privdata, int mask) {
	readQueryFromClient (el, fd, privdata, mask);
}

void replyToClientHandler (aeEventLoop *el, int fd, void *privdata, int mask) {
	sendReplyToClient (el, fd, privdata, mask);
}

/*********************************************************************************
 *********************************************************************************
 * concrete handler implementation
 *********************************************************************************
 */
void readQueryFromClient (aeEventLoop *el, int fd, void *privdata, int mask) {
	REDIS_NOTUSED(el);
    REDIS_NOTUSED(mask);
	
	redisClient *c = (redisClient *)privdata;
	int nread, nreadlen;
	nreadlen = QUERY_CHUNK_BYTE;
	
	nread = read (fd, c->query_buf, nreadlen);
	
	if (-1 == nread) {
		if (errno == EAGAIN) {
			nread = 0;
		} else {
			fprintf (stderr, "Error, read from client(fd=%d): %s\n", fd, strerror (errno));
			freeClient (c);
			return;
		}
	} else if (0 == nread) {
		fprintf (stdout, "Client closed the connection\n");
		freeClient (c);
		return;
	}

	c->query_buf[nread] = '\0';		/* NULL TERM */
	fprintf (stdout, "Receive from client(fd=%d): %s\n", fd, c->query_buf);
	fflush (stdout);

	/*
	 * 服务器写操作只关注于写，不进行缓冲区操作，否则，会不停的重复写
	 */
	/*
	if (aeCreateFileEvent (el, fd, AE_WRITABLE, replyToClientHandler, c) == AE_ERR) {
		fprintf (stderr, "create file event failed : %d\n", __LINE__);
		return;
	}
	*/
	processBuffer (el, c);
}

void sendReplyToClient (aeEventLoop *el, int fd, void *privdata, int mask) {
	REDIS_NOTUSED (el);
	REDIS_NOTUSED (mask);
	
	redisClient *c = (redisClient *)privdata;
	int nwritten = 0;
	char *buf = c->output_buf;
		
	while (c->output_len > 0) {
		nwritten = write(fd, buf, c->output_len);

		if (nwritten <= 0) break;

		buf += nwritten;
		c->output_len -= nwritten;
	}
	
	if (nwritten == -1) {
		if (errno == EAGAIN) {
			nwritten = 0;
		} else {
			fprintf (stderr, "Error writing to client(fd = %d): %s %d\n", fd, 
				strerror (errno), __LINE__);
			freeClient (c);
			return;
		}
	}
}

/* in redis, createClient just had one argument, but here, we add another argument 
 * aeEventLoop, to create the file event
 */
redisClient* createClient (int fd, aeEventLoop *el) {
	if (-1 == fd) {
		return NULL;
	}
	
	redisClient *c = (redisClient *)zmalloc (sizeof (struct redisClient));
	
	if (NULL == c) {
		fprintf (stderr, "createClient failed, %d\n", __LINE__);
		exit (EXIT_FAILURE);
	}

	if (aeCreateFileEvent (el, fd, AE_READABLE, recvFromClientHandler, c) == AE_ERR) {	/* error occured */
		close (fd);

		if (c) {
			zfree (c);
		}
		c = NULL;
	}
	
	c->client_fd = fd;
	memset (c->query_buf, 0, QUERY_CHUNK_BYTE);
	memset (c->output_buf, 0, REPLY_CHUNK_BYTE);
	c->output_len = 0;
	
	/* add client to server, use global variable 'server' directly, is not a good operation */
	if (g_server.current_client_num == MAX_CLIENTS) {
		fprintf (stderr, "too clients, server reject other connections now.\n");
		close (fd);
		zfree (c);
	} else {
		g_server.clients[g_server.current_client_num] = c;
		g_server.current_client_num ++;
	}
	
	return c;
}

void freeClient (redisClient *c) {
	if (c) {
		/* adjust server.clients */
		int fd = c->client_fd, i,j;
		for (i=0; i<g_server.current_client_num; i++) {
			if (fd == g_server.clients[i]->client_fd) {
				break;
			}
		}
		
		for (j=i+1; j<g_server.current_client_num; j++) {
			g_server.clients[i] = g_server.clients[j];
			i++;
		}
		g_server.current_client_num--;
	
		if (c->client_fd != -1) {
			aeDeleteFileEvent (g_server.event_loop, c->client_fd, AE_READABLE);
			aeDeleteFileEvent (g_server.event_loop, c->client_fd, AE_WRITABLE);
		}

		c->client_fd = -1;
		close (c->client_fd);
		memset (c->query_buf, 0, sizeof (c->query_buf));
		memset (c->output_buf, 0, sizeof (c->output_buf));
		
		zfree (c);
		
		c = NULL;
	}
}

void processBuffer (aeEventLoop *el, redisClient *c) {
	sprintf (c->output_buf, "server --> client: %s", c->query_buf);
	c->output_len = strlen (c->output_buf);
	c->output_buf[c->output_len] = '\0';

	fprintf (stdout, "fd=%d: %s\n", c->client_fd, c->output_buf);

	if (aeCreateFileEvent (el, c->client_fd, AE_WRITABLE, replyToClientHandler, c) == AE_ERR) {
		fprintf (stderr, "create file event failed : %d\n", __LINE__);
		return;
	}
}
