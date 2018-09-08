#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "log.h"
#include "http.h"

#define LOG_FILENAME  SERVER_NAME".log"
char* log_file = NULL;

void print_log(struct http_request* req) {
  assert(log_file != NULL);

  //May be NULL
  if(req->ua == NULL)
    req->ua = "???";

  int fd = open(log_file, O_CREAT| O_WRONLY | O_APPEND, 0644);

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

void set_log_dir(char* logDir) {
  assert(log_file == NULL);
  
  char* path = malloc(sizeof(char)*((strlen(logDir)+1+strlen(LOG_FILENAME))));
  strcpy(path,logDir);
  strcat(path,"/");
  strcat(path,LOG_FILENAME);

  log_file = path;
}
