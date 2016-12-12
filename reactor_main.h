#ifndef __REACROT_MAIN_H__
#define __REACTOR_MAIN_H__

#include <stdio.h>
#include <stdlib.h>

#include "ae.h"
#include "anet.h"
#include "ae_event_handler.h"

// redisServer *server;
#define EVENTLOOP_FDSET_INCR 128

#define PORT 24002
#define LISTEN_BACKLOG 20

void InitServer(redisServer *server);

int servCron (aeEventLoop *el, long long id, void *client_data);

#endif
