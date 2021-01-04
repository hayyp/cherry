#include <sys/epoll.h>
#include <stdlib.h>
#include "epoll.h"
#include "log.h"

/* used to store returned information from the ready list */
struct epoll_event *events;

int chry_epoll_create(void)
{
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    events = (struct epoll_event *) malloc(sizeof(struct epoll_event) * MAXEVENTS);
    return epfd;
}

void chry_epoll_add(int epfd, int sockfd, struct epoll_event *ev)
{
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, ev) == -1) {
        perror("EPOLL_CTL_ADD");
        exit(EXIT_FAILURE);
    }
}

void chry_epoll_mod(int epfd, int sockfd, struct epoll_event *ev)
{
    if (epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, ev) == -1) {
        perror("EPOLL_CTL_MOD");
        exit(EXIT_FAILURE);
    }
}

void chry_epoll_del(int epfd, int sockfd, struct epoll_event *ev)
{
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, ev) == -1) {
        perror("EPOLL_CTL_DEL");
        exit(EXIT_FAILURE);
    }
}

int chry_epoll_wait(int epfd, struct epoll_event *events, int maxevents)
{
    int fds = epoll_wait(epfd, events, maxevents, -1);
    if (fds == -1) {
        perror("epoll_wait");
        exit(EXIT_FAILURE);
    }

    return fds;
}
