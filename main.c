#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "http.h"
#include "webserver.h"

#define MAX_VALID_PORT  2<<15
#define MIN_VALID_PORT  0
#define MAX_SIM_CONN    64

int isPortValid(int port) {
	return (port > MIN_VALID_PORT && port < MAX_VALID_PORT);
}

int isValidPath(char* path) {
	if(access(path,W_OK) != 0) {
		perror("isValidPath: access");
		return BOOLEAN_FALSE;
	}
	return BOOLEAN_TRUE;
}

int main(int argc, char *argv[]) {
	fprintf(stderr,"%s -- v%s\n",SERVER_NAME,SERVER_VERSION);
	if(argc != 3) {
		fprintf(stderr,"ERROR: Must specify two arguments.\n");
		fprintf(stderr,"Usage %s PORT DIRECTORY\n",argv[0]);
		fprintf(stderr,"\t* PORT:      Port in which webserver will be listening\n");
		fprintf(stderr,"\t* DIRECTORY: Directory in which the server will scan files to be served\n");
		return EXIT_FAILURE;
	}
	int port = atoi(argv[1]);
	if(!isPortValid(port)) {
		fprintf(stderr,"ERROR: Port specified is not valid(%d)\n",port);
		return EXIT_FAILURE;
	}
	if(!isValidPath(argv[2]))
		return EXIT_FAILURE;

	int pid = 0;
	int listenfd = 0;
	int socketfd = 0;
	int processRequest = BOOLEAN_TRUE;
	static struct sockaddr_in cli_addr;
	static struct sockaddr_in serv_addr;
	socklen_t length = sizeof(cli_addr);

	if((listenfd = socket(AF_INET, SOCK_STREAM,0)) < 0) {
		perror("main: socket");
		return EXIT_FAILURE;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) == -1) {
		perror("main: bind");
		return EXIT_FAILURE;
	}

	if(listen(listenfd,MAX_SIM_CONN) == -1) {
		perror("main: listen");
		return EXIT_FAILURE;
	}

	while(BOOLEAN_TRUE) {
		processRequest = BOOLEAN_TRUE;
		if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0) {
			perror("main: accept");
			processRequest = BOOLEAN_FALSE;
		}

		//Create thread and let it handle request
		pthread_attr_t attr;
		if(pthread_attr_init(&attr) != 0) {
			perror("main: pthread_attr_init");
			return EXIT_FAILURE;
		}
		//Set DETACHED flag, so main thread wont need to call join to
		//clean up thread when finished
		if(pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED) != 0) {
			perror("main: pthread_attr_setdetachstate");
			return EXIT_FAILURE;
		}
		struct req_struct* req = malloc(sizeof(struct req_struct));
		req->socket = socketfd;
		req->dirPath = argv[2];

		pthread_t req_thread;
		if(pthread_create(&req_thread, &attr, &process_web_request, req) != 0) {
  		perror("main: pthread_create");
  		return EXIT_FAILURE;
    }
	}
}
