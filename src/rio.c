/*
 * The RIO package is originally documented in the CS:APPv3 book.
 * */
#include <sys/sendfile.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "rio.h"

void rio_readinitb(struct rio_t *rp, int fd)
{
    rp->rio_fd     = fd;
    rp->rio_left   = 0;           /* remaining characters in rio_buf */
    rp->rio_bufptr = rp->rio_buf;
}

static ssize_t rio_read(struct rio_t *rp, char *usrbuf, size_t n) /* from rio_buf to usrbuf */
{
    while (rp->rio_left <= 0) { /* rio_buf is empty, fill it out */
        rp->rio_left = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf)); /* from fd to rio_buf */
        if (rp->rio_left < 0) {
            if (errno == EAGAIN) {  /* if fd is set to nonblocking and is empty  */
                return -EAGAIN;
            } else if (errno != EINTR) { /* other errors, but not EINTR */
                return -1;
            }
        } else if (rp->rio_left == 0) { /* if fd is set to blocking and is empty */
            return 0;
        } else {
            rp->rio_bufptr = rp->rio_buf;
        }
    }

    int nleft = n; /* bytes to read */
    if (nleft > rp->rio_left) nleft = rp->rio_left;
    memcpy(usrbuf, rp->rio_bufptr, nleft);
    rp->rio_bufptr += nleft;
    rp->rio_left   -= nleft;
    return nleft;
}

ssize_t rio_readlineb(struct rio_t *rp, void *usrbuf, size_t maxlen) /* rio_buf to usrbuf, one line */
{
    int n;
    int rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; ++n) { /* byte by byte */
        if ((rc = rio_read(rp, &c, 1)) == 1) { /* rio_buf to c */
            *bufp++ = c;
            if (c == '\n') {
                ++n;
                break;
            }
        /* rio_buf is empty but errors occur when refilling it */
        } else if (rc == 0) { /* the underlying fd is empty and blocking */
            if (n == 1) {
                return 0;
            } else 
                break;
        } else if (rc == -EAGAIN) { /* the underlying fd is empty and nonblocking */
            return rc;
        } else {
            return -1; /* other errors that is not EINTR */
        }
    }
    *bufp = 0;
    return n - 1;
}

ssize_t rio_readnb(struct rio_t *rp, void *usrbuf, size_t n) /* rio_buf to usrbuf, n bytes */
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nread = rio_read(rp, bufp, nleft)) < 0) {
            return -1;
        } else if (nread == 0) {
            break;
        }
        
        nleft -= nread;
        bufp  += nread;
    }
    return (n - nleft);
}

ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nread = read(fd, bufp, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        } else if (nread == 0) {
            break;
        }

        nleft -= nread;
        bufp  += nread;
    }
    return (n - nleft);
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }

        nleft -= nwritten;
        bufp  += nwritten;
    }
    return n;
}

ssize_t rio_sendfile(int dst, int src, size_t size)
{
    size_t nleft = size;
    ssize_t nwritten;

    while (nleft > 0) {
        if ((nwritten = sendfile(dst, src, NULL, size)) <= 0) {
            if (errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }

        nleft -= nwritten;
    }
    return size;
}
