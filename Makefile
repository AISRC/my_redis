CC=gcc
CFLAGS=-g -Wall -Wunused-function
INCLUDE=-I.

SOURCES=$(shell ls *.c)

REACTOR_SERVER=reactor-server
SERVER_OBJS=ae_event_handler.o ae.o ae_select.o anet.o reactor_main.o

REACTOR_CLI=reactor-cli
CLIENT_OBJS=net.o reactor_cli.o

.PHONY:all
all:$(REACTOR_SERVER) $(REACTOR_CLI)
	@echo ""
	@echo "Hints: compile reactor-server and reactor-cli success"
	@echo ""

sinclude .depend

.PHONY:depend
depend:
	$(CC) -E -MM $(INCLUDE) $(SOURCE) >> .depend

$(REACTOR_SERVER):$(SERVER_OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

$(REACTOR_CLI):$(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

.PHONY:clean
clean:
	$(RM) $(SERVER_OBJS) $(REACTOR_SERVER) $(REACTOR_CLI) $(CLIENT_OBJS)
