#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include "task.h"

#define BACKLOG 20
#define PORT    "3333"

void request_handler(struct task_set *ts, int epfd, int listenfd);
void *get_in_addr(struct sockaddr *sa);
void set_nonblock(int fd);
int setup_listenfd();

#endif
