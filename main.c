#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "http.h"
#include "webserver.h"
#include "args.h"
#include "log.h"

#define MAX_SIM_CONN    64

void help(int argc, char *argv[]) {
	fprintf(stderr,"Usage %s [--port=PORT] [--dir=DIR] [--log=DIR] [--help]\n",argv[0]);
	fprintf(stderr,"\t --port: Port in which webserver will be listening                  (Default: %d)\n",DEFAULT_PORT);
	fprintf(stderr,"\t --dir:  Directory in which the server will scan files to be served (Default: %s)\n",DEFAULT_DIR);
	fprintf(stderr,"\t --log:  Directory in which the server will save log file           (Default: %s)\n",DEFAULT_DIR);
	fprintf(stderr,"\t --help: Print this help and exit\n");
}

int main(int argc, char *argv[]) {
	fprintf(stderr,"%s -- v%s\n",SERVER_NAME,SERVER_VERSION);

	if(!parseArgs(argc,argv))
    return EXIT_FAILURE;

  if(show_help()) {
    help(argc,argv);
    return EXIT_SUCCESS;
  }

	set_log_dir(get_log_dir());
	int pid = 0;
	int listenfd = 0;
	int socketfd = 0;
	int processRequest = BOOLEAN_TRUE;
	static struct sockaddr_in cli_addr;
	static struct sockaddr_in serv_addr;
	socklen_t length = sizeof(cli_addr);

	printf("Listening on port %d\n",get_port());

	if((listenfd = socket(AF_INET, SOCK_STREAM,0)) < 0) {
		perror("main: socket");
		return EXIT_FAILURE;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(get_port());

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

		//Get ip of the request
    char* ipAddress = malloc(sizeof(char)*INET_ADDRSTRLEN);
    if(inet_ntop(AF_INET, &(cli_addr.sin_addr), ipAddress, INET_ADDRSTRLEN) == NULL)
      perror("main: inet_ntop");

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
		req->dirPath = get_working_dir();
		req->ipAddress = ipAddress;

		pthread_t req_thread;
		if(pthread_create(&req_thread, &attr, &process_web_request, req) != 0) {
  		perror("main: pthread_create");
  		return EXIT_FAILURE;
    }
	}
}
