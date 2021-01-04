#ifndef HTTP_H
#define HTTP_H

struct mime_t {
  const char *type;
  const char *value;
};

void handle_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void handle_request(int fd);

#endif
