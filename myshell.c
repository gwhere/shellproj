#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

extern char **get_line();
int exit_status = EXIT_SUCCESS;
const int REDIR_IN = -1;
const int REDIR_OUT = 1;
const int MAX_PATH_LEN = 80;

struct exec_info {
  char* prog_name;
  char*** args;
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
  if (args[1] == NULL){
    abs_path = getenv("HOME");
  } else 
    abs_path = strdup(args[1]);
  return abs_path;
}

void check_flags(struct exec_info *info, struct exec_info *info2) {
  //printf("   DEBUG: entering check_flags\n");
  int i, j = -1;
  int k, l;
  char **args = *(info->args);
  char **p_args2 = NULL;
  char **p_args = NULL;
  info->prog_name = args[0];
  for (i = 0; args[i] != NULL; i++) {
    j++;
  }

  //printf("   DEBUG: testing for ampersand\n");
  /* assign the index of the last argument to j */
  if(strcmp(args[j], "&") == 0) {
    info->bkgrd = 1;
    j--;
  }
  

  //printf("   DEBUG: about to enter p_args for loop\n");
  for(i = 0; i <= j; i++) {
    //printf("   DEBUG: added %s to p_args\n",p_args[i]);
    if(strcmp(args[i], "<") == 0) {
      info->redirect = REDIR_IN;
    }
    if(strcmp(args[i], ">") == 0) {
      info->redirect = REDIR_OUT;
    }
    if(info->redirect != 0) {
      info->file_name = args[i+1];
      j-=2;
      break;
    }
    if(strcmp(args[i], "|") == 0) {
      if(args[i + 1] == NULL) { /*error*/ break;}
      p_args2 = malloc((j - i) * sizeof(char*));
      l = 0;
      for(k=i + 1; k<=j; k++)
      { 
        p_args2[l] = args[k];
        //printf("args2[%d] = %s\n", l, p_args2[l]);
        l++;
      }
      info2->args = &p_args2;
      info2->prog_name = p_args2[0];
      j = i - 1;
      //printf("info2->prog_name: %s, info2->args[0]: %s\n", info2->prog_name, *(info2->args)[0]);
      break;
    }
  }


  /* set args to only arguments needed for execution */
  p_args = malloc((j+1) * sizeof(char*));
  
  for(i=0; i<=j; i++) p_args[i] = args[i];
  info->args = &p_args;
 
}

void test_info(struct exec_info *info) {
  printf("bkgrd = %d, redirect = %d, file_name = %s\n", info->bkgrd, info->redirect, info->file_name);
}

void execute(struct exec_info *info, struct exec_info *info2) {
  int status, cpid, pipefd[2];
  char **args = *(info->args);
  char *prog = info->prog_name;

  if (pipe(pipefd) == -1)
  {
    perror("Failed to create pipe.");
    exit(EXIT_FAILURE);
  }

  cpid = fork();
  if(cpid!=0)
    {
      if(info2->prog_name != NULL) close(pipefd[1]);
      if(info->bkgrd) signal(SIGCHLD,SIG_IGN);
      else waitpid(-1, &status, 0);
    }
  else {
    if(info->redirect == REDIR_IN)
    {
      freopen(info->file_name, "r", stdin);
    } else if(info->redirect == REDIR_OUT)
    {
      freopen(info->file_name, "w", stdout);
    }
    
    if(info2->prog_name != NULL) dup2(pipefd[1], 1);
    if (execvp(prog, args) == -1) {
      fprintf(stderr, "%s: Invalid command.\n", prog);
      exit(exit_status);
    }
  }

  /* Handle piping */
  if(info2->prog_name != NULL) {
    cpid = fork();
    if(cpid!=0) {
      waitpid(-1, &status, 0);
    } else {
      args = *(info2->args);
      prog = info2->prog_name;
      dup2(pipefd[0], 0);
      if (execvp(prog, args) == -1) {
        fprintf(stderr, "%s: Invalid command.\n", prog);
        exit(exit_status);
      }
    }
  }
}


void parse_cmd(struct exec_info *info) {
  struct exec_info *info2 = cons_info();
  char currdir[MAX_PATH_LEN]; 
  char **args = *(info->args);
  
  if (args[0] == NULL) return;
  
  if (strcmp(args[0], "exit") == 0) { 
    exit(exit_status);
  } 
  
  else if (strcmp(args[0], "cd") == 0) {
    char *dirname = get_abs_path(args);
    chdir(dirname);
    getcwd(currdir, MAX_PATH_LEN);
  }
  
  else {
    //printf("   DEBUG: entered else\n");
    check_flags(info, info2);
    execute(info, info2);
  }
}

char* clip_path(char* pathname) {
  int endindex = strlen(pathname) -1;
  char clipped[endindex + 1];
  int i;
  for (i = 0; i < strlen(pathname); ++i){
    if (pathname[i] == '/') {
      strncpy(clipped, pathname + i + 1, endindex);
    }
  }
  pathname = strdup(clipped);
  return pathname;
}

main() { 
  int i;
  char **args;

  while(1) {
    char buf[80];
    printf("[myshell %s]$ ", clip_path(getcwd(buf, sizeof(buf))));
    struct exec_info *info = cons_info();
    args = get_line();
    info->args = &args;
    parse_cmd(info);
  }
}

