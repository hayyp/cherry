#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include "http.h"
#include "rio.h"
#include "log.h"

#define LONGMAX 8192 /* that is, 8KB */
#define SHORTMAX 512
#define root "../statics"     /* where to find static files such as index.html */

static struct mime_t mime_list[] = {
    {".css",  "text/css"},
    {".html", "text/html"},
    {".png",  "image/png"},
    {".jpg",  "image/jepg"},
    {".jepg", "image/jepg"},
    {".gz",   "application/x-gzip"},
    {".tar",  "application/x-tar"}
};
 
void handle_request(int fd);
void handle_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

static void serve_static(int fd, char *filename, int filesize);
static void parse_uri(char *uri, char *filename, char *query_str);
static const char *get_file_type(const char *extension);

void handle_request(int fd)
{
    struct rio_t rio;
    char method[LONGMAX], uri[SHORTMAX], version[SHORTMAX];
    char buf[LONGMAX];

    char filename[SHORTMAX];
    struct stat sbuf;

    rio_readinitb(&rio, fd);

    while (1) {
        int rc = rio_readlineb(&rio, buf, LONGMAX);
        if (rc < 0 && rc != -EAGAIN) {
            log_error("error occurs when reading request");
            exit(EXIT_FAILURE);
        } else if (rc == -EAGAIN) {
            break;
        }
        
        sscanf(buf, "%s %s %s", method, uri, version); // for example, GET / HTTP/1.1 
        log_info("%s %s %s", method, uri, version);

        if (strcasecmp(method, "GET")) { // if not GET
            handle_error(fd, method, "501", "Not Implemented", "HTTP Method Not Supported");
            return;
        }

        parse_uri(uri, filename, NULL);

        if (stat(filename, &sbuf) < 0) {
            handle_error(fd, filename, "404", "Not Found", "The requested file cannot be found");
            return;
        }

        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            handle_error(fd, filename, "403", "Forbidden", "Access not authorized");
            return;
        }

        serve_static(fd, filename, sbuf.st_size);
    }
}

void handle_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char header[LONGMAX], body[LONGMAX];

    sprintf(body, 
        "<html><title>ERROR</title>\r\n"
        "%s: %s\r\n"
        "<p>%s: %s\r\n</p>"
        "<hr><em>The CHERRY Web Server</em></body></html>\r\n",
        errnum, shortmsg, cause, longmsg);

    const char *file_type;
    const char *extension = strrchr(cause, '.');
    file_type = get_file_type(extension);

    sprintf(header, 
        "HTTP/1.1 %s %s\r\n"
        "Server: CHERRY\r\n"
        "Content-type: %s\r\n"
        "Connection: keep-alive\r\n"
        "Keep-Alive: timeout=5, max=1000\r\n"
        "Content-length: %d\r\n\r\n",
        errnum, file_type, shortmsg, (int) strlen(body));
    
    rio_writen(fd, header, strlen(header));
    rio_writen(fd, body, strlen(body));
}

static void parse_uri(char *uri, char *filename, char *query_str)
{
    strcpy(filename, root);
    strcat(filename, uri);

    char *last_comp = strrchr(filename, '/');
    char *last_dot  = strrchr(last_comp, '.');

    if (last_dot == NULL && filename[strlen(filename) - 1] != '/') {
        strcat(filename, "/");
    }

    if (uri[strlen(uri) - 1] == '/' || filename[strlen(filename) - 1] == '/') {
        strcat(filename, "index.html");
    }

    return;
}

static void serve_static(int fd, char *filename, int filesize)
{
    char header[LONGMAX];
    int n;

    const char *file_type;
    const char *extension = strrchr(filename, '.');
    file_type = get_file_type(extension);

    sprintf(header,
            "HTTP/1.1 200 OK\r\n"
            "Server: CHERRY\r\n"
            "Content-length: %d\r\n"
            "Connection: keep-alive\r\n"
            "Keep-Alive: timeout=5, max=1000\r\n"
            "Content-type: %s\r\n\r\n",
            filesize, file_type);

    n = rio_writen(fd, header, strlen(header));
    if (n != strlen(header)) {
        log_error("rio_writen error");
        exit(EXIT_FAILURE);
    }

    int srcfd = open(filename, O_RDONLY, 0);
    if (srcfd <= 2) {
        log_error("open");
        exit(EXIT_FAILURE);
    }


    n = rio_sendfile(fd, srcfd, filesize);
    if (n == -1) {
        perror("sendfile");
        exit(EXIT_FAILURE);
    }

    close(srcfd);
}

static const char *get_file_type(const char *extension)
{
    if (extension == NULL) {
        return "text/plain";
    }

    int i;
    for (i = 0; mime_list[i].type != NULL; ++i) {
        if (strcmp(extension, mime_list[i].type) == 0)
            return mime_list[i].value;
    }

    return mime_list[i].value;
}
