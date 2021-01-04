#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include "epoll.h"
#include "utils.h"
#include "http.h"
#include "task.h"
#include "log.h"

void request_handler(struct task_set *ts, int epfd, int listenfd)
{
    char s[INET6_ADDRSTRLEN] = {0};
    int cli_fd;
    struct sockaddr_storage cli_addr;
    socklen_t sin_size = sizeof(cli_addr);

    struct epoll_event ev;

    int fd = task_get(ts);
    if (fd == 0) {
        log_error("attemp to read an empty task set");
        exit(EXIT_FAILURE);
    }

    if (fd == listenfd) {
        for (;;) {
            cli_fd = accept(fd, (struct sockaddr *) &cli_addr, &sin_size);
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
        handle_request(fd);
    }
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

void set_nonblock(int fd)
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

int setup_listenfd()
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
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(servinfo_list);

    return sockfd;
}
