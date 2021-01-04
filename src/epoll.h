#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include "task.h"

#define MAXEVENTS 1024

/* used to store returned information from the ready list */
extern struct epoll_event *events;

/* used in state machine */
extern void (*ep_wait_strategy[2])();

/* basically just a wrapper above the epoll API with error handling */
int  chry_epoll_create(void);
void chry_epoll_add(int epfd, int sockfd, struct epoll_event *ev);
void chry_epoll_mod(int epfd, int sockfd, struct epoll_event *ev);
void chry_epoll_del(int epfd, int sockfd, struct epoll_event *ev);
int  chry_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

/* stateful epoll_wait for state machine */
void chry_epoll_wait_block(struct task_set *ts, int epfd, struct epoll_event *events, int maxevents);
void chry_epoll_wait_nonblock(struct task_set *ts, int epfd, struct epoll_event *events, int maxevents);

#endif
