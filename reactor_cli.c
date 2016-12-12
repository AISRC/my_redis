/**************************************************
    > File name: reactor_cli.c
    > Author: wzhenyu
    > mail: mblrwuzy@gmail.com
    > Create Time: 2016-12-06 14:49
    > Copyright 2015 Wzhenyu, all rights reserved. 
****************************************************/
#include "net.h"
#include <string.h>

#define DEFAULT_ADDR "127.0.0.1"
#define PORT 24002

/* write and get info from server */
static void issueCommandRepeat (char *buf, redisContext *c) {
	if (!buf) return;

	char *p = buf;
	int buf_len = strlen (buf);
	int nwritten = 0, nread = 0;
	int wdone = 0, rdone = 0;

	do {
		nwritten = write (c->fd, p, strlen (p));
		if (-1 == nwritten) {	/* fd had been marked non-blocking */
			if (errno == EAGAIN) {
				/* TODO later again */
			} else {
				fprintf (stderr, "write failed, %d\n", __LINE__);
				exit (EXIT_FAILURE);
			}
		} else if (nwritten > 0) {
			if (nwritten == buf_len) {
				wdone = 1;
			} else {
				p = p + nwritten;
				buf_len -= nwritten;
			}
		}
	} while (!wdone);

	memset (c->buf, 0, sizeof (c->buf));
	do {
		nread = read (c->fd, c->buf, CLI_QUERY_CHUNK_BYTE);
		if (-1 == nread) {
			if (errno == EAGAIN) {
				/* TODO later again */
			} else {
				fprintf (stderr, "read error, %d\n", __LINE__);
				exit (EXIT_FAILURE);
			}
		} else if (0 == nread) {
			fprintf (stdout, "Server closed the connection (fd = %d)\n", c->fd);
			exit (EXIT_SUCCESS);
		} else if (nread > 0) {
			rdone = 1;
		}
	} while (!rdone);
}

int main(int argc, char* argv[])
{
	char buf[CLI_QUERY_CHUNK_BYTE] = {0};
	int buf_len = 0;

	redisContext *c = redisConnect (DEFAULT_ADDR, PORT);

	while (fgets(buf, CLI_QUERY_CHUNK_BYTE, stdin) != NULL) {
		buf_len = strlen (buf);	

		/* cut off \n */
		if (buf[buf_len-1] == '\n') {
			buf[buf_len-1] = '\0';
			buf_len--;
		}

		if (buf[0] != '\0') {	/* input string is not NULL */
			if (strcasecmp (buf, "quit") == 0
					|| strcasecmp (buf, "exit") == 0) {
				exit (EXIT_SUCCESS);
			} else {
				issueCommandRepeat (buf, c);
			}
		}

		fprintf (stdout, "%s\n", c->buf);
	}

	redisFreeContext (c);

    return 0;
}

