#ifndef __HTTP__
#define __HTTP__

#define SERVER_NAME 		"swebs"
#define SERVER_VERSION	"1.0"
#define MIME_HTML			 	"text/html"

/* HTTP CODE */

#define HTTP_CODE_200  200
#define HTTP_CODE_400  400
#define HTTP_CODE_403  403
#define HTTP_CODE_404  404
#define HTTP_CODE_500  500

static char RESPONSE_200[] = "HTTP/1.1 200 OK\r\n";
static char RESPONSE_400[] = "HTTP/1.1 400 Bad Request\r\n";
static char RESPONSE_403[] = "HTTP/1.1 403 Forbidden\r\n";
static char RESPONSE_404[] = "HTTP/1.1 404 Not Found\r\n";
static char RESPONSE_500[] = "HTTP/1.1 500 Internal Server Error\r\n";

#define RESPONSE_200_SIZE 17
#define RESPONSE_400_SIZE 26
#define RESPONSE_403_SIZE 24
#define RESPONSE_404_SIZE 24
#define RESPONSE_500_SIZE 36

/* END HTTP CODE */

/* HEADERS */

#define HEADER_SERVER 				"Server: "SERVER_NAME"/"SERVER_VERSION" (Unix)\r\n"
#define HEADER_KEEP_ALIVE			"Keep-Alive: timeout=5\r\n"
#define HEADER_CONNECTION			"Connection: Keep-Alive\r\n"
#define HEADER_CONTENTTYPE		"Content-Type: "
#define HEADER_CONTENTLENGHT	"Content-Length: "

#define headerServerSize 				26
#define headerKeepAliveSize 		23
#define headerConnectionSize 		24
#define headerContentTypeSize 	14
#define headerContentLengthSize 16

/* END HEADERS */

struct Header {
	int size;
	char* content;
};

typedef struct Header *Header;

#endif
