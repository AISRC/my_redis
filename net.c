/**************************************************
    > File name: net.c
    > Author: wzhenyu
    > mail: mblrwuzy@gmail.com
    > Create Time: 2016-12-06 15:49
    > Copyright 2015 Wzhenyu, all rights reserved. 
****************************************************/
#include "net.h"

redisContext *redisConnect (const char *ip, int port) {
	redisContext *c = (redisContext *)zmalloc (sizeof (redisContext));

	if (NULL == c) {
		fprintf (stderr, "redisConnect failed, %d\n", __LINE__);
		exit (EXIT_FAILURE);
	}

	redisContextConnectTcp (c, ip, port, NULL);

	return c;
}

static int redisSetBlocking (redisContext *c, int blocking) {
	int flags;

	if ((flags = fcntl (c->fd, F_GETFL)) == -1) {
		fprintf (stderr, "fcntl(F_GETFL) failed, %d\n", __LINE__);

		if (c && c->fd>0) {
			close (c->fd);
			c->fd = -1;
		}

		return CLI_ERR;
	}

	if (blocking) {
		flags &= ~O_NONBLOCK;
	} else {
		flags |= O_NONBLOCK;
	}

	if (fcntl (c->fd, F_SETFL, flags) == -1) {
		fprintf (stderr, "fcntl(F_SETFL) failed, %d\n", __LINE__);

		if (c && c->fd>0) {
			close (c->fd);
			c->fd = -1;
		}

		return CLI_ERR;
	}

	return CLI_OK;
}

static int _redisContextConnectTcp (redisContext *c, const char *addr, 
		int port, const struct timeval *timeout) {
	int s, rv;
	char _port[6];	/* strlen ("65535") */

	struct addrinfo hints, *servinfo, *p;

	snprintf (_port, 6, "%d", port);
	memset (&hints, 0, sizeof (hints));

	hints.ai_family = AF_INET;	/* IPV4 */
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo (addr, _port, &hints, &servinfo)) != 0)	{	/* success return 0 */
		hints.ai_family = AF_INET6;		/* not ipv4, try ipv6 */

		if ((rv = getaddrinfo (addr, _port, &hints, &servinfo)) != 0) {
			fprintf (stderr, "getaddrinfo failed: %s, %d\n", gai_strerror(rv), __LINE__);
			return CLI_ERR;
		}
	}

	for (p=servinfo; p!=NULL; p=p->ai_next) {
		if ((s = socket (p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			continue;
		}

		c->fd = s;
		if (redisSetBlocking (c, 0) != CLI_OK) {
			goto ERR;
		}

		if (connect (s, p->ai_addr, p->ai_addrlen) == -1) {
			if (errno == EHOSTUNREACH) { /* connected failed */
				if (c && c->fd>0) {
					close (c->fd);
					c->fd = -1;
				}

				continue;
			} else if (errno == EINPROGRESS) {
				 /* This is ok. */
			} else {
				fprintf (stderr, "connect failed:%s %d\n", strerror(errno), __LINE__);
				goto ERR;
			}
		}

		if (redisSetBlocking (c, 1) != CLI_OK) {
			goto ERR;
		}

		rv = CLI_OK;
		goto OK;
	}

	if (NULL == p) {
		fprintf (stderr, "Can't create socket: %s, %d\n", strerror(errno), __LINE__);
		goto ERR;
	}

ERR:
	rv = CLI_ERR;
OK:
	freeaddrinfo (servinfo);
	return rv;
}

int redisContextConnectTcp (redisContext *c, const char *ip, int port, 
		const struct timeval * timeout) {
	return _redisContextConnectTcp (c, ip, port, timeout);
}

void redisFreeContext (redisContext *c) {
	if (c->fd > 0) {
		close (c->fd);
	}

	memset (c->buf, 0, CLI_QUERY_CHUNK_BYTE);
	zfree (c);
}
