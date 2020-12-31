#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include "http.h"
#include "log.h"

#define BACKLOG 20
#define PORT    "3333"

static void *get_in_addr(struct sockaddr *sa);
static int create_listenfd();

int main()
{
    int sockfd = create_listenfd();
    char s[INET6_ADDRSTRLEN] = {0};
    int cli_fd;
    struct sockaddr_storage cli_addr;
    socklen_t sin_size = sizeof(cli_addr);
    
    log_info("CHERRY started, now running on port " PORT);

    while (1) {
        cli_fd = accept(sockfd, (struct sockaddr *)&cli_addr, &sin_size); 
        if (cli_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(cli_addr.ss_family, get_in_addr((struct sockaddr *) &cli_addr), s, sizeof(s));
        log_info("Client IP %s", s);

        handle_request(cli_fd);
        close(cli_fd);
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

static int create_listenfd()
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
