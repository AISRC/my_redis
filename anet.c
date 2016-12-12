#include "anet.h"

/*
 * err: error message
 * s: server socket file descriptor
 * sa: store ip, port and so on when accept a connection from a client
 * len: sizeof (sa)
 * function: loop until accept and get an useable fd
 */
static int anetGenericAccept (int s, struct sockaddr *sa, socklen_t *len) {
	int fd;
	while (1) {
		fd = accept (s, sa, len);
		
		/* if fd is EAGAIN, return ANET_ERR, otherwise, will go into dead loop,
		 * because if no clients connect to server this moment, fd (non_block) 
		 * will accept and return EAGAIN */
		if (-1 == fd) {
			if (errno == EINTR)
				continue;
			else {
				fprintf (stderr, "accept: %s\n", strerror(errno));
				return ANET_ERR;
			}
		}
		break;
	}
	
	return fd;
}

/*
 * err: err message
 * serversock: socket file descriptor
 * ip: store the ip address accepted (client's ip)
 * ip_len: length of ip
 * port:
 */
int anetTcpAccept (int serversock, char* ip, size_t ip_len, int *port) {
	int fd;
	struct sockaddr_storage sa;
	socklen_t salen = sizeof (sa);
	
	if ((fd = anetGenericAccept (serversock, (struct sockaddr*)&sa, &salen)) == -1)
		return ANET_ERR;
		
	if (sa.ss_family == AF_INET) {
		struct sockaddr_in *s = (struct sockaddr_in *)&sa;
		if (ip)
			inet_ntop (AF_INET, (void*)&(s->sin_addr), ip, ip_len);
		if (port)
			*port = ntohs (s->sin_port);
	} else {
		struct sockaddr_in6 * s = (struct sockaddr_in6 *)&sa;
		
		if (ip)
			inet_ntop (AF_INET6, (void*)&(s->sin6_addr), ip, ip_len);
		if (port)
			*port = ntohs (s->sin6_port);	/* imcompatible type */
	}
	
	return fd;
}

static int anetSetReuseAddr (int sockfd) {
	int yes = 1;
	if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (yes)) == -1) {
		fprintf (stderr, "setsockopt SO_REUSEADDR: %s, %s : %d\n", strerror (errno),
			__FILE__, __LINE__);
		return ANET_ERR;
	}
	return ANET_OK;
}

static int anetListenBind (int sockfd, struct sockaddr * addr, socklen_t addrlen, int backlog) {
	if (bind (sockfd, addr, addrlen) == -1) {
		fprintf (stderr, "bind : %s, %s : %d\n", strerror (errno), __FILE__, __LINE__);
		close (sockfd);
		return ANET_ERR;
	}
	
	if (listen (sockfd, backlog) == -1) {
		fprintf (stderr, "listen : %s, %s : %d\n", strerror (errno), 
			__FILE__, __LINE__);
		close (sockfd);
		return ANET_ERR;
	}
	
	return ANET_OK;
}

static int _anetTcpServer (int port, char * bindaddr, int flag, int backlog) {
	int s, rv;
	char _port[6];	/* in Linux, max port is 65535, so max length of string port(65535) is 5, don't forget NULL TERM */
	struct addrinfo hints, *servinfo, *p;
	
	snprintf (_port, 6, "%d", port);
	memset (&hints, 0, sizeof (hints));
	hints.ai_family = flag;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;		/* No effect when bindaddr != NULL, in this case, AI_PASSIVE flag is ignored */
	
	/* hints as a filter */
	if ((rv = getaddrinfo (bindaddr, _port, &hints, &servinfo)) != 0)	 { /* error */
		fprintf (stderr, "getaddrinfo: %s, %s : %d\n", gai_strerror(rv), __FILE__, __LINE__);
		return ANET_ERR;
	}
	
	for (p=servinfo; p!=NULL; p=p->ai_next) {
		if ((s = socket (p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			continue;	/* addrinfo is unusable, try the next one */
		}
		
		/* set socket option reuse */
		if (anetSetReuseAddr (s) == ANET_ERR) {
			goto ERR;
		}
		
		if (anetListenBind (s, p->ai_addr, p->ai_addrlen, backlog) == ANET_ERR) {
			goto ERR;
		}
		goto END;
	}
	
	if (NULL == p) {
		fprintf (stderr, "Unable to bind socket\n");
		goto ERR;
	}
ERR:
	s = ANET_ERR;
END:
	freeaddrinfo (servinfo);
	return s;
}

/*
 * bindaddr: can be NULL
 * backlog: the length of listened queue by listened socket
 */
int anetTcpServer (int port, char *bindaddr, int backlog) {
	return _anetTcpServer (port, bindaddr, AF_INET, backlog);	// IPV4
}

/*
 * use getaddrinfo to get one or more addrinfo structures, and return an useable one from them
 */

/*
 * set fd O_NONBLOCK
 */
int anetNonBlock (int fd) {
	return anetSetBlock (fd, 1);
}

int anetSetBlock (int fd, int non_block) {
	int flags;
	
	if ((flags = fcntl (fd, F_GETFL)) == -1) {
		fprintf (stderr, "fcntl (fd, F_GETFL) error: %s\n", strerror (errno));
		return ANET_ERR;
	}
	
	if (non_block) {
		flags |= O_NONBLOCK;
	} else {
		flags &= ~O_NONBLOCK;
	}
	
	if ((fcntl (fd, F_SETFL, flags)) == -1) {
		fprintf (stderr, "fcntl (fd, F_SETFL, flags) error : %s\n", strerror (errno));
		return ANET_ERR;
	}
	
	return ANET_OK;
}
