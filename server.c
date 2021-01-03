#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include "epoll.h"
#include "http.h"
#include "log.h"

#define BACKLOG 20
#define PORT    "3333"

static int  setup_listenfd();
static void set_nonblock(int fd);
static void *get_in_addr(struct sockaddr *sa);

int main()
{
    int listenfd = setup_listenfd();
    set_nonblock(listenfd);

    log_info("CHERRY started, now running on port " PORT);

    int epfd = chry_epoll_create();
    struct epoll_event ev;
    ev.data.fd = listenfd; // data returned along with the ready list
    ev.events = EPOLLIN | EPOLLET;
    chry_epoll_add(epfd, listenfd, &ev);

    char s[INET6_ADDRSTRLEN] = {0};
    int cli_fd;
    struct sockaddr_storage cli_addr;
    socklen_t sin_size = sizeof(cli_addr);
    
    while (1) {
        int ready_fds;
        ready_fds = chry_epoll_wait(epfd, events, MAXEVENTS);

        int i;
        for (i = 0; i < ready_fds; ++i) {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))) {
                log_error("epoll_wait: wrong events");
                close(events[i].data.fd);
                continue;
            }

            if (listenfd == events[i].data.fd) {
                for (;;) {
                    cli_fd = accept(listenfd, (struct sockaddr *) &cli_addr, &sin_size);
                    if (cli_fd == -1) {
                        if ((errno == EAGAIN) ||
                            (errno == EWOULDBLOCK)) { // has handled all requests
                            break;
                        } else {
                            log_error("accept");
                            break; // see man accept for more errors
                        }
                    }

                    inet_ntop(cli_addr.ss_family, get_in_addr((struct sockaddr *) &cli_addr), s, sizeof(s));
                    log_info("client %s", s);

                    set_nonblock(cli_fd);

                    ev.data.fd = cli_fd;
                    ev.events = EPOLLIN | EPOLLET;

                    chry_epoll_add(epfd, cli_fd, &ev);
                }
            } else {
                handle_request(events[i].data.fd);
                close(events[i].data.fd);
            }
        }
    }

    return 0;
}

static void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

static int setup_listenfd()
{
    struct addrinfo hints, *servinfo, *servinfo_list;
    int sockfd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int err;
    if ((err = getaddrinfo(NULL, PORT, &hints, &servinfo_list))) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        exit(1);
    }

    for (servinfo = servinfo_list; servinfo != NULL; servinfo = servinfo->ai_next) {
        if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
            close(sockfd);
            perror("bind");
            continue;
        }

        break;
    }

    if (servinfo == NULL) {
        fprintf(stderr, "failed to bind socket\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    freeaddrinfo(servinfo_list);

    return sockfd;
}

static void set_nonblock(int fd)
{
    int fileflags;

    if ((fileflags = fcntl(fd, F_GETFL, 0)) == -1) {
        perror("F_GETFL");
        exit(EXIT_FAILURE);
    }

    if ((fileflags = fcntl(fd, F_SETFL, fileflags | O_NONBLOCK) == -1)) {
        perror("F_SETFL");
        exit(EXIT_FAILURE);
    }
}
