#include "structs.h"
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

void builtin_exit(Command *cmd, int *shell_status) {
  if (cmd -> argc > 1) {
    fprintf(stderr, "exit: too many arguments\n");
    *shell_status = 1;
  }
  else {
    exit(*shell_status);
  }
}

void builtin_cd(Command *cmd, int *shell_status) {
  if (cmd -> argc > 2) {
    fprintf(stderr, "cd: too many argumens\n");
    *shell_status = 1;
  }
  else if (cmd -> argc == 1) {
    const char *home = getenv("HOME");
    if (home != NULL) {
      if (chdir(home) != 0) {
        perror("cd");
        *shell_status = 1;
      }
    }
  }
  else {
    if (chdir(cmd -> args[1]) != 0) {
      perror("cd");
      *shell_status = 1;
    }
  }
}

void builtin_path(Command *cmd, int *shell_status, SearchPath *path) {
  if (cmd -> argc == 1) {
    for (int i = 0; i < path -> count; i++) {
      printf("%s\n", path -> dirs[i]);
    }
    return;
  }

  int ncount = cmd -> argc - 1;

  for (int i = 0; i < path -> count; i++) {
    free(path -> dirs[i]);
  }

  free(path -> dirs);
  path -> dirs = malloc(sizeof(char *) * ncount);

  for (int i = 0; i < ncount; i++) {
    path -> dirs[i] = strdup(cmd -> args[1 + i]);
  }
  
  path -> count = ncount;
}