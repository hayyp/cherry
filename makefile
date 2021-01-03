CC=gcc
CFLAGS= -Wall -Wextra -DLOG_USE_COLOR 

.PHONY: clean

cherry: server.c log.c rio.c http.c epoll.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm cherry a.out
