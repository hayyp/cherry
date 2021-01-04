/*
 * The RIO package is originally documented in the CS:APPv3 book.
 * */
#ifndef RIO_H
#define RIO_H

#define RIO_BUFSIZE 8192

/* 
 * containing: 
 *     a) a buffer that can serve as an internal cache to save IO related system calls
 *     b) related information that can be used to track progress/position
 * */
struct rio_t {
    int   rio_fd;
    int   rio_left;
    char *rio_bufptr;
    char  rio_buf[RIO_BUFSIZE];
};

void    rio_readinitb   (struct rio_t *rp, int fd);
ssize_t rio_readnb      (struct rio_t *rp, void *usrbuf, size_t n);      /* wrapper functions that use rio_read */
ssize_t rio_readlineb   (struct rio_t *rp, void *usrbuf, size_t maxlen); /* to read from the rio_buf cache buffer */

ssize_t rio_readn (int fd, void *usrbuf, size_t n);  /* fd => usrbuf */
ssize_t rio_writen(int fd, void *usrbuf, size_t n);

ssize_t rio_sendfile(int dst, int src, size_t size);

#endif
