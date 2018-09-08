#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "args.h"
#include "webserver.h"

#define ARG_STR_PORT      "port"
#define ARG_STR_DIR       "dir"
#define ARG_STR_LOG       "log"
#define ARG_STR_HELP      "help"
#define ARG_CHAR_PORT     'p'
#define ARG_CHAR_DIR      'd'
#define ARG_CHAR_LOG      'l'
#define ARG_CHAR_HELP     'h'

#define MAX_VALID_PORT  2<<15
#define MIN_VALID_PORT  0

static struct args_struct args;
struct args_struct {
  int port_specified;
  int working_dir_specified;
  int log_dir_specified;

  int port;
  int help_flag;
  char* working_dir;
  char* log_dir;
};

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

int parseArgs(int argc, char* argv[]) {
  int c;
  int digit_optind = 0;
  int option_index = 0;
  opterr = 0;

  args.port_specified = BOOLEAN_FALSE;
  args.working_dir_specified = BOOLEAN_FALSE;
  args.log_dir_specified = BOOLEAN_FALSE;

  args.help_flag = BOOLEAN_FALSE;
  args.port = DEFAULT_PORT;
  args.log_dir = NULL;
  args.working_dir = NULL;

  static struct option long_options[] = {
      {ARG_STR_PORT,  required_argument,   0, ARG_CHAR_PORT    },
      {ARG_STR_DIR,   required_argument,   0, ARG_CHAR_DIR     },
      {ARG_STR_LOG,   required_argument,   0, ARG_CHAR_LOG     },
      {ARG_STR_HELP,  no_argument,         0, ARG_CHAR_HELP    },
      {0, 0, 0, 0}
  };

  c = getopt_long(argc, argv,"",long_options, &option_index);

  while (c != -1) {
    if(c == ARG_CHAR_PORT) {
      if(args.port_specified) {
        fprintf(stderr,"ERROR: Port specified more than once\n");
        return BOOLEAN_FALSE;
      }
      args.port_specified = BOOLEAN_TRUE;
      args.port = atoi(optarg);
    }
    else if(c == ARG_CHAR_DIR) {
      if(args.working_dir_specified) {
        fprintf(stderr,"ERROR: Working dir specified more than once\n");
        return BOOLEAN_FALSE;
      }
      args.working_dir_specified = BOOLEAN_TRUE;
      args.working_dir = optarg;
    }
    else if(c == ARG_CHAR_LOG) {
      if(args.log_dir_specified) {
        fprintf(stderr,"ERROR: Log dir specified more than once\n");
        return BOOLEAN_FALSE;
      }
      args.log_dir_specified = BOOLEAN_TRUE;
      args.log_dir = optarg;
    }
    else if(c == ARG_CHAR_HELP) {
      if(args.help_flag) {
         fprintf(stderr,"ERROR: Help option specified more than once\n");
         return BOOLEAN_FALSE;
      }
      args.help_flag = BOOLEAN_TRUE;
    }
    else if(c == '?') {
      fprintf(stderr,"WARNING: Invalid options\n");
      args.help_flag  = BOOLEAN_TRUE;
      break;
    }
    else
      fprintf(stderr,"Bug at line number %d in file %s\n", __LINE__, __FILE__);

    option_index = 0;
    c = getopt_long(argc, argv,"",long_options, &option_index);
  }

  if (optind < argc) {
    fprintf(stderr,"WARNING: Invalid options\n");
    args.help_flag  = BOOLEAN_TRUE;
  }

  if(args.working_dir == NULL)
    args.working_dir = DEFAULT_DIR;

  if(args.log_dir == NULL)
    args.log_dir = DEFAULT_DIR;

  if(!isPortValid(args.port)) {
  	fprintf(stderr,"ERROR: Port specified is not valid(%d)\n",args.port);
  	return BOOLEAN_FALSE;
  }

  if(!isValidPath(args.working_dir))
		return BOOLEAN_FALSE;

  if(!isValidPath(args.log_dir))
		return BOOLEAN_FALSE;

  return BOOLEAN_TRUE;
}

int show_help() {
  return args.help_flag;
}
int get_port() {
  return args.port;
}
char* get_working_dir() {
  return args.working_dir;
}
char* get_log_dir() {
  return args.log_dir;
}
