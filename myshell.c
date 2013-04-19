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
  char *file_name;
};

struct exec_info *cons_info(){
  struct exec_info *info = malloc(sizeof(struct exec_info));
  info->bkgrd = 0;
  info->redirect = 0;
  return info;
}

char* get_abs_path(char** args) {
  char* abs_path;
  int i;
  if (args[1] == NULL){
    abs_path = getenv("HOME");
  } else 
    abs_path = strdup(args[1]);
  return abs_path;
}

void check_flags(struct exec_info *info) {
  int i, j = -1;
  char **args = info->args;
  for (i = 0; args[i] != NULL; i++) {
    j++;
  }
  /* assign the index of the last argument to j */
  if(strcmp(args[j], "&") == 0)
  {
    info->bkgrd = 1;
    j--;
  }

  for(i = 0; i <= j; i++) {
    if(strcmp(args[i], "<") == 0) {
      info->redirect = REDIR_IN;
    }
    if(strcmp(args[i], ">") == 0) {
      info->redirect = REDIR_OUT;
    }
    if(info->redirect != 0) {
      info->file_name = args[i+1];
      break;
    }
  }
}

void test_info(struct exec_info *info) {
  printf("bkgrd = %d, redirect = %d, file_name = %s\n", info->bkgrd, info->redirect, info->file_name);
}

void parse_cmd(struct exec_info *info) {
  char currdir[MAX_PATH_LEN]; 
  char **args = info->args;
  
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
    struct exec_info *info = cons_info();
    info->args = get_line();
    parse_cmd(info); 
    check_flags(info);
    
    test_info(info);
  }
}

