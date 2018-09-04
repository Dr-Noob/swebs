#ifndef __WEBSERVER__
#define __WEBSERVER__

#define BOOLEAN_TRUE 		1
#define BOOLEAN_FALSE 	0

struct req_struct {
  int socket;
  char* dirPath;
  char* ipAddress;
};

struct http_request {
  char* method;
  char* resource;
  char* ip;
  char* ua;
};

int write_all(int fd, char* buf,int bytesToWrite);
void* process_web_request(void* param);
int send_page(int socket, int http_code, int fd, int lenght, char* extension);

#endif
