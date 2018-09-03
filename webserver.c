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

#define BUFSIZE       8096
#define ERROR           42
#define LOG             44
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
  int code_size = 0;

  switch (http_code) {
    case HTTP_CODE_200:
      code_size = RESPONSE_200_SIZE;
      code_response = RESPONSE_200;
      break;
    case HTTP_CODE_400:
      page = PAGE_400;
      code_size = RESPONSE_400_SIZE;
      code_response = RESPONSE_400;
      lenght = strlen(page);
      extension = MIME_HTML;
      break;
    case HTTP_CODE_403:
      page = PAGE_403;
      code_size = RESPONSE_403_SIZE;
      code_response = RESPONSE_403;
      lenght = strlen(page);
      extension = MIME_HTML;
      break;
    case HTTP_CODE_404:
      page = PAGE_404;
      code_size = RESPONSE_404_SIZE;
      code_response = RESPONSE_404;
      lenght = strlen(page);
      extension = MIME_HTML;
      break;
    case HTTP_CODE_500:
      page = PAGE_500;
      code_size = RESPONSE_500_SIZE;
      code_response = RESPONSE_500;
      lenght = strlen(page);
      extension = MIME_HTML;
      break;
    default:
      printf("FATAL ERROR: Invalid http code %d\n",http_code);
  }
  assert(code_size != 0);

  int digits = floor(log10(abs(lenght))) + 1;
  int headerSize = code_size + headerServerSize + headerKeepAliveSize +
                   headerConnectionSize + headerContentTypeSize + strlen(extension) + 2 +
                   headerContentLengthSize + digits + 2 + 2;

  Header header = malloc(sizeof(struct Header));
  header->size = headerSize;
  header->content = malloc(sizeof(char)* header->size + 1);
  memset(header->content,0,header->size);
  snprintf(header->content, sizeof(char)* header->size + 1,
                                                           "%s"
                                                           HEADER_SERVER
                                                           HEADER_CONTENTTYPE"%s\r\n"
                                                           HEADER_KEEP_ALIVE
                                                           HEADER_CONNECTION
                                                           HEADER_CONTENTLENGHT"%d\r\n"
                                                           "\r\n"
                                                           ,code_response,extension,lenght);
  if(http_code == HTTP_CODE_200) {
    write_all(socket,header->content,header->size);

    //Read file and write it in 8KB block size
    int bytes_read = 0;
    int block_size = 2<<12;
    char* buf = malloc(sizeof(char)*block_size);
    memset(buf, 0, sizeof(char)*block_size);

    while (  (bytes_read = read(fd, buf, block_size)) > 0 )
            write_all(socket,buf,bytes_read);

    free(buf);
    free(header->content);
    free(header);

    if(bytes_read == -1) {
      perror("code200: read");
      send_page(socket, HTTP_CODE_500, 0, 0, NULL);
    }
  }
  else {
    int responseSize = lenght + header->size;
    char response[responseSize];
    memset(response, 0, responseSize);
    strcpy(response, header->content);
    memcpy(response+header->size,page,lenght);

    write_all(socket,response,responseSize);
    free(header->content);
    free(header);
  }

  return EXIT_SUCCESS;
}

void process_web_request(int socket,char* dirPath) {
  int buf_size = 4096;
  int bytes_read = 0;
  int flick = 128;
  int count = 0;
  char buf[buf_size];

  int end = BOOLEAN_FALSE;
  int socketClosed = BOOLEAN_FALSE;

  char* method = NULL;
  char* resource = NULL;

  printf("Socket accepted(%d)\n",socket);
  fflush(stdout);

  do {
    count = 0;
    bytes_read = 0;
    end = BOOLEAN_FALSE;
    memset(buf, 0, buf_size);

    while (!end && (bytes_read = read(socket, buf+count, flick)) > 0) {
      count += bytes_read;
      end = messageFinished(buf,count);
    }

    if(bytes_read == -1) {
      perror("process_web_request: read");
      send_page(socket, HTTP_CODE_500, 0, 0, NULL);
      socketClosed = BOOLEAN_TRUE;
    }

    if(bytes_read == 0) {
      //Connection closed by the client
      socketClosed = BOOLEAN_TRUE;
    }
    if(strlen(buf) != 0) {
      method = getMethod(buf);
      resource = getResource(buf);

      if(method == NULL || resource == NULL) {
        printf("ERROR: Malformed request\n");
        printf("[%s]\n",buf);
        fflush(stdout);
        send_page(socket, HTTP_CODE_400, 0, 0, NULL);
        socketClosed = BOOLEAN_TRUE;
        if(method != NULL)free(method);
        if(resource != NULL)free(resource);
      }
      else {
        printf("Requested by %s resource %s\n",method,resource);
        fflush(stdout);

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
  fflush(stdout);
  exit(0);
}
