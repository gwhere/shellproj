#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

extern char **get_line();
int exit_status = EXIT_SUCCESS;
const int REDIR_IN = -1;
const int REDIR_OUT = 1;
const int MAX_PATH_LEN = 80;

struct exec_info {
  char* prog_name;
  char** args;
  int bkgrd;
  int redirect; 
};

char* get_abs_path(char** args) {
  char* abs_path;
  int i;
  if (args[1] == NULL){
    abs_path = getenv("HOME");
  } else 
    abs_path = strdup(args[1]);
  return abs_path;
}

void parse_cmd(char** args) {
  char currdir[MAX_PATH_LEN]; 
  
  if (strcmp(args[0], "exit") == 0) { 
    exit(exit_status);
  } 
  else if (strcmp(args[0], "cd") == 0) {
    char* dirname = get_abs_path(args);
    chdir(dirname);
    getcwd(currdir, MAX_PATH_LEN);
    printf("Current Directory: %s\n", currdir);      
  }
}

main() { 
  int i;
  char **args;

  while(1) {
    args = get_line();
    parse_cmd(args); 
  }
}
