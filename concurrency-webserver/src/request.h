#ifndef __REQUEST_H__

#include "io_helper.h"

#define MAXBUF (8192)

typedef struct HTTPRequest {
    int is_static;
    struct stat sbuf;
    char buf[MAXBUF];
    char method[MAXBUF];
    char uri[MAXBUF];
    char version[MAXBUF];
    char filename[MAXBUF];
    char cgiargs[MAXBUF];
} HTTPRequest;

void request_handle(int fd);
void request_parse(int fd, HTTPRequest *req);
void request_handle_without_parse(int fd, HTTPRequest *req);

#endif // __REQUEST_H__
