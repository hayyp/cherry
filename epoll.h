#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>

#define MAXEVENTS 1024

/* basically just a wrapper above the epoll API with error handling */
int  chry_epoll_create(void);
void chry_epoll_add(int epfd, int sockfd, struct epoll_event *ev);
void chry_epoll_mod(int epfd, int sockfd, struct epoll_event *ev);
void chry_epoll_del(int epfd, int sockfd, struct epoll_event *ev);
int  chry_epoll_wait(int epfd, struct epoll_event *events, int maxevents);

/* used to store returned information from the ready list */
extern struct epoll_event *events;

#endif
