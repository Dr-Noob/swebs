#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <arpa/inet.h>

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
      code500(socket);
      socketClosed = BOOLEAN_TRUE;
    }

    if(bytes_read == 0) {
      //Conexion cerrada por el cliente
      socketClosed = BOOLEAN_TRUE;
    }
    if(strlen(buf) != 0) {
      method = getMethod(buf);
      resource = getResource(buf);

      if(method == NULL || resource == NULL) {
        printf("ERROR: Malformed request\n");
        printf("[%s]\n",buf);
        fflush(stdout);
        code400(socket);
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
          code400(socket);

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
