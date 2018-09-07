#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#include "webserver.h"
#include "get.h"
#include "http.h"

#define EXTENSIONS_SIZE 34
#define MIME_PLAIN_TEX "text/plain"

struct {
	char *ext;
	char *filetype;
} extensions [] =
	{
	{"avi"	, "video/x-msvideo" },
	{"bin"	, "application/octet-stream" },
	{"css"	, "text/css" },
	{"csv"	, "text/csv" },
	{"doc"	, "application/msword" },
	{"epub"	, "application/epub+zip" },
	{"gif"	, "image/gif" },
	{"htm"	, MIME_HTML },
	{"html"	, MIME_HTML },
	{"ico"	, "image/x-icon" },
	{"jar"	, "application/java-archive" },
	{"jpg"	, "image/jpeg" },
	{"jpeg"	, "image/jpeg" },
	{"js"		, "application/javascript" },
	{"json"	,	"application/json" },
	{"mpeg"	, "video/mpeg" },
	{"odp"	, "application/vnd.oasis.opendocument.presentation" },
	{"ods"	, "application/vnd.oasis.opendocument.spreadsheet" },
	{"odt"	, "application/vnd.oasis.opendocument.text" },
	{"pdf"	, "application/pdf" },
	{"rar"	, "application/x-rar-compressed" },
	{"sh"		, "application/x-sh" },
	{"apk"	, "image/svg+xml" },
	{"tar"	, "application/x-tar" },
	{"xls"	, "application/vnd.ms-excel" },
	{"xml"	, "application/xml" },
	{"zip"	, "application/zip" },
	{"7z"		, "application/x-7z-compressed" },
	{"txt"	, MIME_PLAIN_TEX },
	{"c"		, MIME_PLAIN_TEX },
	{"cpp"	, MIME_PLAIN_TEX },
	{"h"		, MIME_PLAIN_TEX },
	{"hpp"	, MIME_PLAIN_TEX },
	{"py"		, MIME_PLAIN_TEX },
	};

//Returns true if string str starts with string pre
int startsWith(const char *pre, const char *str)
{
    int preLenght = strlen(pre);
		int strLenght = strlen(str);

		if(preLenght > strLenght)
			return BOOLEAN_FALSE;

    return strncmp(pre, str, preLenght) == 0;
}

//Returns true if file in filePath cant be found
//by means of directory workingDir
int forbidden(char* filePath, char* workingDir) {
	char* absolutePathFile = realpath(filePath, NULL);
	char* absolutePathDir = realpath(workingDir, NULL);

	if(absolutePathFile == NULL || absolutePathDir == NULL)
	{
		perror("forbidden");
		return BOOLEAN_TRUE;
	}

	int ret = !startsWith(absolutePathDir, absolutePathFile);
	free(absolutePathFile);
	free(absolutePathDir);

	return ret;
}

//Returns file extension by means of its
//filename, or NULL if wouldnt have extension
char* getExtension(char* fileName) {
	//add two to check extension of hidden files
	//(those that start by .)
	char* point = strrchr(fileName+2, '.');
	if(point == NULL)
		return NULL;
	return point+1;
}

void handleGet(int socket, char*dirPath, char* resource) {
	if(strcmp(resource,"/") == 0) {
		//We've been asked for default
		//resource, so we redefine it
		//to index.html
		char newResource[11] = "/index.html";
		resource = newResource;
	}

	//Open file to check if it exists
	char path[sizeof(char)*(strlen(dirPath)+strlen(resource))];
	strcpy(path,dirPath);
	strcat(path,resource);
	FILE *file = fopen(path, "r");

	//If it does not exist, return 404
	if(file == NULL) {
		perror("handleGet: fopen");
		send_page(socket, HTTP_CODE_404, 0, 0, NULL);
		return;
	}

	//If exists, we check we have read permissions
	//and that it is a regular file, so we create
	//st struct to get its size
	int fd = fileno(file);
	struct stat st;
	if(stat(path,&st) == -1) {
		perror("handleGet: stat");
		perror(path);
		fclose(file);
		return;
	}
	if(!S_ISREG(st.st_mode)) {
		fprintf(stderr,"ERROR: '%s' is not a file\n",path);
		send_page(socket, HTTP_CODE_404, 0, 0, NULL);
		fclose(file);
		return;
	}

	//We check that file is not in a higher
	//hirerachy's directory
	if(forbidden(path, dirPath)) {
		fprintf(stderr,"ERROR: '%s' is forbidden\n",path);
		send_page(socket, HTTP_CODE_403, 0, 0, NULL);
		return;
	}

	//Get extension and check if its supported
	char* extension = getExtension(resource);
	char* fileType = NULL;

	//If it does not have extension, we assume it
	//is plain text
	if(extension == NULL)
		fileType = MIME_PLAIN_TEX;
	else {
		for(int i=0;i<EXTENSIONS_SIZE;i++) {
			if(strcmp(extensions[i].ext,extension) == 0) {
				fileType = extensions[i].filetype;
				break;
			}
		}

		if(fileType == NULL)  {
			fprintf(stderr,"ERROR: Extension .%s is not supported\n",extension);
			send_page(socket, HTTP_CODE_400, 0, 0, NULL);
			fclose(file);
			return;
		}
	}

	send_page(socket, HTTP_CODE_200, fd, st.st_size, fileType);
	fclose(file);
}
