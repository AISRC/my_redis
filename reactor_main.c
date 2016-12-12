#include "reactor_main.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

redisServer g_server;

void initServer (redisServer *server) {
	server->pid = getpid ();
	server->event_loop = aeCreateEventLoop (EVENTLOOP_FDSET_INCR);
	server->tcp_backlog = LISTEN_BACKLOG;
	server->server_fd = -1;
	server->port = PORT;
	server->current_client_num = 0;
	
	int i;
	for (i=0; i<MAX_CLIENTS; i++) {
		server->clients[i] = NULL;
	}
	
	if ((server->server_fd = listenToPort (server->port, server->tcp_backlog)) == AE_ERR) {
		exit (EXIT_FAILURE);
	}
	
	if (aeCreateTimeEvent (server->event_loop, 5000, servCron, NULL, NULL) == AE_ERR) {
		fprintf (stderr, "can not create time event servCron : %d\n", __LINE__);
		*((char*)-1) = 'x';		// set addr of 0xFFFFFFFF to char 'x', invalid operation, 
								//will cause a fatal error (segmentation fault)
		/* exit (EXIT_FAILURE); */
	}
	
	if (aeCreateFileEvent (server->event_loop, server->server_fd, AE_READABLE, acceptTcpHandler, 
		NULL) == AE_ERR) {
		fprintf (stderr, "can not create file event acceptTcpHandler : %d\n", __LINE__);
		exit (EXIT_FAILURE);
	}
}

int servCron (aeEventLoop *el, long long id, void *client_data) {
	/* printf current time every interval */
	time_t mtime;
	struct tm *m_tm;

	mtime = time (NULL);
	m_tm = localtime (&mtime);

	fprintf (stdout, "Current time is \"%4d-%02d-%02d %02d:%02d:%02d\"\n", m_tm->tm_year+1900, m_tm->tm_mon,
			m_tm->tm_mday, m_tm->tm_hour, m_tm->tm_min, m_tm->tm_sec);

	return 1;
}

extern redisServer server;

int main (int argc, char* argv[]) {
	initServer (&g_server);
	
	aeMain (g_server.event_loop);
	aeDeleteEventLoop (g_server.event_loop);
	return 0;
}
