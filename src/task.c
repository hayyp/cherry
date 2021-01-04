#include <stdlib.h>
#include "task.h"
#include "log.h"

struct task *task_create(int fd)
{
    struct task *taskp = (struct task *) malloc(sizeof(struct task));
    taskp->fd = fd;
    taskp->next = NULL;
    return taskp;
}

struct task_set *task_set_init()
{
    struct task_set *ts = (struct task_set *) malloc(sizeof(struct task_set));
    ts->front = ts->rear = NULL;
    ts->task_set_len = 0;
    return ts;
}

void task_add(struct task_set *ts, int fd)
{
    struct task *taskp = task_create(fd);
    ts->task_set_len += 1;

    if (ts->rear == NULL) {
        ts->front = ts->rear = taskp;
        return;
    }

    ts->rear->next = taskp;
    ts->rear = taskp;
}

int task_get(struct task_set *ts)
{
    if (ts->front == NULL) return 0;

    struct task *taskp = ts->front;
    ts->front = ts->front->next;

    if (ts->front == NULL) ts->rear = NULL;
    ts->task_set_len--;

    int fd = taskp->fd;
    free(taskp);
    return fd;
}
