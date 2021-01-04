#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include "epoll.h"
#include "task.h"
#include "log.h"

/* used to store returned information from the ready list */
struct epoll_event *events;
void (*ep_wait_strategy[2])() = {chry_epoll_wait_block, chry_epoll_wait_nonblock};

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

int chry_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    int fds = epoll_wait(epfd, events, maxevents, timeout);
    if (fds == -1) {
        perror("epoll_wait");
        exit(EXIT_FAILURE);
    }

    return fds;
}

void chry_epoll_wait_block(struct task_set *ts, int epfd, struct epoll_event *events, int maxevents)
/* stateful epoll_wait, used in a state machine */
{
    int ready_fds;
    ready_fds = chry_epoll_wait(epfd, events, maxevents, -1);

    int i;
    for (i = 0; i < ready_fds; ++i) {
        if ((events[i].events & EPOLLERR) ||
            (events[i].events & EPOLLHUP) ||
            (!(events[i].events & EPOLLIN))) {
            log_error("epoll_wait: unexpected event detected, closing affected fd");
            close(events[i].data.fd);
            continue;
        }

        task_add(ts, events[i].data.fd);
    }
}

void chry_epoll_wait_nonblock(struct task_set *ts, int epfd, struct epoll_event *events, int maxevents)
/* stateful epoll_wait, used in a state machine */
{
    int ready_fds;
    ready_fds = chry_epoll_wait(epfd, events, maxevents, 5);

    if (ready_fds == 0) {
        return;
    } else {
        int i;
        for (i = 0; i < ready_fds; ++i) {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))) {
                log_error("epoll_wait: unexpected event detected, closing affected fd");
                close(events[i].data.fd);
                continue;
            }

            task_add(ts, events[i].data.fd);
        }
    }
}
