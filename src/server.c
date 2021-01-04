#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include "utils.h"
#include "epoll.h"
#include "task.h"
#include "http.h"
#include "log.h"
#include "fsm.h"

int main()
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL)) {
        log_error("failed to install signal handler");
        return 0;
    }

    int epfd = chry_epoll_create();

    int listenfd = setup_listenfd();
    set_nonblock(listenfd);

    struct epoll_event ev;
    ev.data.fd = listenfd;
    ev.events = EPOLLIN | EPOLLET;
    chry_epoll_add(epfd, listenfd, &ev);

    log_info("CHERRY started, now running on port " PORT);
    
    /* task_set is used to store fds with new connections
     * such that they can be handled later instead of
     * on-the-spot
     * */
    struct task_set *ts = task_set_init();
    state_t task_set_state = STATE_EMPTY;

    while (1) {
        switch (task_set_state) {
            case STATE_EMPTY:
                /* when there is no tasks to do, epoll_wait is allowed to
                 * block until there are more connections, which will be 
                 * immediately added to task_set to avoid starvation
                 * */
                ep_wait_strategy[STATE_EMPTY](ts, epfd, events, MAXEVENTS);
                break;
            case STATE_NONEMPTY:
                /* when there are tasks in the task_set, epoll_wait should not
                 * block the control flow. Whether or not there are new connections,
                 * the control flow should return to the request handling routine ASAP
                 * */
                ep_wait_strategy[STATE_NONEMPTY](ts, epfd, events, MAXEVENTS); /* nonblocking, still have jobs waiting */
                break;
            default:
                break;
        }

        if (ts->task_set_len > 0) 
            request_handler(ts, epfd, listenfd);

        if (ts->task_set_len == 0) 
            task_set_state = STATE_EMPTY;
        else
            task_set_state = STATE_NONEMPTY;
    }

    return 0;
}
