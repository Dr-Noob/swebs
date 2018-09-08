#ifndef __ARGS__
#define __ARGS__

#define INVALID_PORT -1
#define DEFAULT_DIR     "."
#define DEFAULT_PORT    80

int parseArgs(int argc, char* argv[]);
int show_help();
int get_port();
char* get_working_dir();
char* get_log_dir();

#endif
