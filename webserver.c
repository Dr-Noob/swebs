#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <assert.h>
#include <arpa/inet.h>

#include "http.h"
#include "get.h"
#include "webserver.h"

#define BUF_SIZE     2<<12
#define GET_METHOD   "GET"

/* PAGES */

static char* PAGE_400 =
"<html>\n"
"<head><title>400 Bad Request</title></head>\n"
"<body bgcolor=\"white\">\n"
"<center><h1>400 Bad Request</h1></center>\n"
"<hr><center>"SERVER_NAME" "SERVER_VERSION"</center>\n"
"</body>\n"
"</html>\n\n"
;

static char* PAGE_403 =
"<html>\n"
"<head><title>403 Forbidden</title></head>\n"
"<body bgcolor=\"white\">\n"
"<center><h1>403 Forbidden</h1></center>\n"
"<hr><center>"SERVER_NAME" "SERVER_VERSION"</center>\n"
"</body>\n"
"</html>\n\n"
;

static char* PAGE_404 =
"<html>\n"
"<head><title>404 Not Found</title></head>\n"
"<body bgcolor=\"white\">\n"
"<center><h1>404 Not Found</h1></center>\n"
"<hr><center>"SERVER_NAME" "SERVER_VERSION"</center>\n"
"</body>\n"
"</html>\n\n"
;

static char* PAGE_500 =
"<html>\n"
"<head><title>500 Internal Server Error</title></head>\n"
"<body bgcolor=\"white\">\n"
"<center><h1>500 Internal Server Error</h1></center>\n"
"<hr><center>"SERVER_NAME" "SERVER_VERSION"</center>\n"
"</body>\n"
"</html>\n\n"
;

/* END PAGES */

int write_all(int fd, char* buf,int bytesToWrite) {
  int bytesWritten = 0;
  int ret = 0;
  do {
    ret = write(fd,buf+bytesWritten,bytesToWrite-bytesWritten);
    if(ret != -1)bytesWritten+=ret;
    else {
      perror("write_all: write");
      return BOOLEAN_FALSE;
    }
  } while(bytesWritten != bytesToWrite);

  return BOOLEAN_TRUE;
}


int messageFinished(char* message, int size) {
  return strstr(message, "\r\n\r\n") != NULL;
}

//Returns a string with the method in
//HTTP message, or NULL if not found
char* getMethod(char* httpMessage) {
  char* end = strstr (httpMessage," ");
  if(end == NULL)
    return NULL;

  int lenght = end-httpMessage;
  char* m = malloc(sizeof(char)*(lenght+1));
  memset(m,0,sizeof(char)*(lenght+1));
  strncpy(m, httpMessage, lenght);
  return m;
}

//Returns a string with the resource in
//HTTP message, or NULL if not found
char* getResource(char* httpMessage) {
  //Start of resource
  char* start = strstr (httpMessage," ");
  if(start == NULL)
    return NULL;

  //Set pointer after space
  start++;

  //End of resource
  char* end = strstr (start, " ");

  if(end == NULL)
    return NULL;

  int lenght = end-start;
  if(lenght <= 0)return NULL;

  //Copy and return string
  char* m = malloc(sizeof(char)*(lenght+1));
  memset(m,0,sizeof(char)*(lenght+1));
  strncpy(m, start, lenght);
  return m;
}

int send_page(int socket, int http_code, int fd, int lenght, char* extension) {
  char* page = NULL;
  char* code_response = NULL;
  int headerSize = 0;

  switch (http_code) {
    case HTTP_CODE_200:
      headerSize = HEADER_200_SIZE;
      code_response = RESPONSE_200;
      break;
    case HTTP_CODE_400:
      page = PAGE_400;
      headerSize = HEADER_400_SIZE;
      code_response = RESPONSE_400;
      lenght = strlen(page);
      extension = MIME_HTML;
      break;
    case HTTP_CODE_403:
      page = PAGE_403;
      headerSize = HEADER_403_SIZE;
      code_response = RESPONSE_403;
      lenght = strlen(page);
      extension = MIME_HTML;
      break;
    case HTTP_CODE_404:
      page = PAGE_404;
      headerSize = HEADER_404_SIZE;
      code_response = RESPONSE_404;
      lenght = strlen(page);
      extension = MIME_HTML;
      break;
    case HTTP_CODE_500:
      page = PAGE_500;
      headerSize = HEADER_500_SIZE;
      code_response = RESPONSE_500;
      lenght = strlen(page);
      extension = MIME_HTML;
      break;
    default:
      printf("FATAL ERROR: Invalid http code %d\n",http_code);
  }
  assert(headerSize != 0);

  int digits = floor(log10(abs(lenght))) + 1;
  //We add the size of extension and lenght of digits
  //to the fixed-size of the HTTP code selected on
  //previous switch-case
  headerSize += strlen(extension) + digits;
  char content[sizeof(char)* headerSize + 1];
  snprintf(content, sizeof(char)* headerSize + 1,
                                                "%s"
                                                HEADER_SERVER
                                                HEADER_CONTENTTYPE"%s\r\n"
                                                HEADER_KEEP_ALIVE
                                                HEADER_CONNECTION
                                                HEADER_CONTENTLENGHT"%d\r\n"
                                                "\r\n"
                                                ,code_response,extension,lenght);
  if(http_code == HTTP_CODE_200) {
    //First, write header
    write_all(socket,content,headerSize);

    //Now read file and write it to the socket
    int bytes_read = 0;
    int write_status = 0;
    char buf[sizeof(char)*BUF_SIZE];
    do {
      bytes_read = read(fd, buf, BUF_SIZE);
      write_status = write_all(socket,buf,bytes_read);
    } while(bytes_read > 0 && write_status);

    if(bytes_read == -1) {
      perror("send_page: HTTP_CODE_200: read");
      send_page(socket, HTTP_CODE_500, 0, 0, NULL);
      return EXIT_FAILURE;
    }
    else if(!write_status)
      return EXIT_FAILURE;
    else
      return EXIT_SUCCESS;
  }
  else {
    //We append static page to the header
    int responseSize = lenght + headerSize;
    char response[responseSize];
    memcpy(response, content, headerSize);
    memcpy(response+headerSize,page,lenght);

    return write_all(socket,response,responseSize);
  }
}

void process_web_request(int socket,char* dirPath) {
  int bytes_read = 0;
  int count = 0;
  int end = BOOLEAN_FALSE;
  int socketClosed = BOOLEAN_FALSE;

  char buf[BUF_SIZE];
  char* method = NULL;
  char* resource = NULL;

  printf("Socket accepted\n");

  do {

    do {
      bytes_read = read(socket, buf, BUF_SIZE);
      end = messageFinished(buf,count);
    } while(!end && bytes_read > 0);

    if(bytes_read == -1) {
      perror("process_web_request: read");
      send_page(socket, HTTP_CODE_500, 0, 0, NULL);
      socketClosed = BOOLEAN_TRUE;
    }
    else if(bytes_read == 0) {
      //Connection closed by the client
      socketClosed = BOOLEAN_TRUE;
    }
    else {
      //We have data on buf
      method = getMethod(buf);
      resource = getResource(buf);

      if(method == NULL || resource == NULL) {
        printf("ERROR: Malformed request\n");
        printf("[%s]\n",buf);

        send_page(socket, HTTP_CODE_400, 0, 0, NULL);
        socketClosed = BOOLEAN_TRUE;
        if(method != NULL)free(method);
        if(resource != NULL)free(resource);
      }
      else {
        printf("Requested by %s resource %s\n",method,resource);

        if(strcmp(method,GET_METHOD) == 0)
          handleGet(socket, dirPath, resource);
        else
          send_page(socket, HTTP_CODE_400, 0, 0, NULL);

        free(resource);
        free(method);
      }
    }

  } while(!socketClosed);

  close(socket);
  printf("Socket closed(%d)\n",socket);
  exit(0);
}
