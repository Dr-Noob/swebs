#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "log.h"
#include "http.h"

#define LOG_FILENAME  SERVER_NAME".log"

void print_log(struct http_request* req) {
  //May be NULL
  if(req->ua == NULL)
    req->ua = "???";

  int fd = open(LOG_FILENAME, O_CREAT| O_WRONLY | O_APPEND, 0644);

  if (fd == -1)
    perror("print_log: open");
  else {
    int size = strlen(req->ip)+strlen(req->ua)+strlen(req->method)+strlen(req->resource)+2+1+2+2;
    char message[(sizeof(char)*(size+1))];
    snprintf(message, size+1, "[%s]-[%s]%s:%s\n", req->ip,req->ua,req->method,req->resource);
    write_all(fd,message,size);
    close(fd);
  }
}
