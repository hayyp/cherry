#ifndef TASK_H
#define TASK_H

struct task {
    int fd;
    struct task *next;
};

struct task_set {
    int task_set_len;
    struct task *front, *rear;
};

struct task *task_create(int fd);
struct task_set *task_set_init();
void task_add(struct task_set *ts, int fd);
int task_get(struct task_set *ts);

#endif
