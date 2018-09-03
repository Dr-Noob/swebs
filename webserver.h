#ifndef __WEBSERVER__
#define __WEBSERVER__

#define BOOLEAN_TRUE 		1
#define BOOLEAN_FALSE 	0

void process_web_request(int socket,char* dirPath);
int send_page(int socket, int http_code, int fd, int lenght, char* extension);

#endif
