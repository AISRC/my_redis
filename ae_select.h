#ifndef __AE_SELECT_H__
#define __AE_SELECT_H__

#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "ae.h"
#include "zmalloc.h"

typedef struct aeApiState {
	    fd_set rfds, wfds;
		    /* We need to have a copy of the fd sets as it's not safe to reuse
			 *      *      * FD sets after select(). */
		    fd_set _rfds, _wfds;
} aeApiState;

int aeApiCreate (aeEventLoop *el);
int aeApiResize (aeEventLoop *el, int resize);
void aeApiFree (aeEventLoop *el);
int aeApiAddEvent (aeEventLoop *el, int fd, int mask);
void aeApiDelEvent (aeEventLoop *el, int fd, int mask);
int aeApiPoll (aeEventLoop *el, struct timeval *tvp);
char *aeApiName (void);
#endif
