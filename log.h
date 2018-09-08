#ifndef __LOG__
#define __LOG__

#include "webserver.h"

void print_log(struct http_request* req);
void set_log_dir(char* logDir);

#endif
