#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_VALID_PORT  2<<15
#define MIN_VALID_PORT  0
#define MAX_SIM_CONN    64
#define BOOLEAN_TRUE 		1
#define BOOLEAN_FALSE 	0

void process_web_request(int socket,char* dirPath) {
	printf("request on %s\n",dirPath);
	exit(0);
}

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
		perror("main");
		return EXIT_FAILURE;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) == -1) {
		perror("main");
		return EXIT_FAILURE;
	}

	if(listen(listenfd,MAX_SIM_CONN) == -1) {
		perror("main");
		return EXIT_FAILURE;
	}

	while(BOOLEAN_TRUE) {
		processRequest = BOOLEAN_TRUE;
		if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0) {
			perror("main");
			processRequest = BOOLEAN_FALSE;
		}
		if((pid = fork()) < 0) {
			perror("main");
			processRequest = BOOLEAN_FALSE;
		}
		if(processRequest) {
			if(pid == 0) {
				close(listenfd);
				process_web_request(socketfd, argv[2]);
			} else {
				close(socketfd);
			}
		}
	}
}
