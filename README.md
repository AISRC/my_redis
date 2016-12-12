# my_redis
Use Makefile to build.

Reactor-server is server and reactor-cli is client. The demultiplexing API I used is "select", so, you could also use epoll or kqueue, just to encapsulate it like ae_select.c does, and you can learn that from redis source code, and even directly use ae_select.c/ae_epoll.c/ae_kqueue.c from redis source code.

If you have any problems, please let me know (mblrwuzy@gmail.com).
