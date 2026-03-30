#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "structs.h"
#include "builtins.h"

#define USAGE "Usage: bcshell [BATCHFILE]\n"

CommandList parse_line(char *line);
Command parse_command(char *segment);
bool is_builtin(const char *name);
void builtin_handler(Command *cmd, int *shell_status, SearchPath *path);
void redirect(Command *cmd);
int run_command(Command *command, SearchPath *path, int *shell_status);
void handle_command(Command *command, int *shell_status, SearchPath *path);
void run_cmdlist(CommandList *cmdlist, int *shell_status, SearchPath *path);
int run_command_wait(Command *command, SearchPath *path);
char *find_executable(Command *command, SearchPath *path);
void free_command(Command *cmd);
void free_cmdlist(CommandList *cmdlist);

int main(int argc, char *argv[]) {
  SearchPath path;
  path.dirs = malloc(sizeof(char *));
  path.dirs[0] = strdup("/bin");
  path.count = 1;

  int shell_status = 0;
  FILE *input;
  bool nobatchfile = false;

  if (argc >= 3) {
    fputs(USAGE, stderr);
    exit(1);
  }
  else if (argc == 2) {
    input = fopen(argv[1], "r");
    if (input == NULL) {
      fprintf(stderr, "%s: file not found\n", argv[1]);
      exit(1);
    }
  }
  else {
    input = stdin;
    nobatchfile = true;
  }

  if (nobatchfile) {
    printf("bcshell> ");
  }

  char *line = NULL;
  size_t n = 0;

  while (getline(&line, &n, input) != -1) {
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
      line[len - 1] = '\0';
    }

    if (line[0] != '\0') {
      CommandList cmdlist = parse_line(line);

      if (cmdlist.count == 1 && cmdlist.commands[0].name != NULL && is_builtin(cmdlist.commands[0].name)) {
        builtin_handler(&cmdlist.commands[0], &shell_status, &path);
      }
      else {
        run_cmdlist(&cmdlist, &shell_status, &path);
      }
      free_cmdlist(&cmdlist);
    }

    if (nobatchfile) {
      printf("bcshell> ");
    }
  }

  free(line);

  if (input != stdin) {
    fclose(input);
  }

  for (int i = 0; i < path.count; i++) {
    free(path.dirs[i]);
  }
  free(path.dirs);

  return shell_status;
}

CommandList parse_line(char *line) {
  CommandList cmdlist;
  cmdlist.count = 0;
  cmdlist.commands = NULL;

  if (strchr(line, '&')) {
    cmdlist.delimiter = '&';
  }
  else if (strchr(line, ';')) {
    cmdlist.delimiter = ';';
  }
  else {
    cmdlist.delimiter = '\0';
  }

  char *copy = strdup(line);
  char *remaining = copy;
  char *segment;

  if (cmdlist.delimiter == '\0') {
    cmdlist.commands = malloc(sizeof(Command));
    cmdlist.commands[0] = parse_command(copy);
    cmdlist.count = 1;
  }
  else {
    char delim[2] = { cmdlist.delimiter, '\0' };
    while ((segment = strsep(&remaining, delim)) != NULL) {
      cmdlist.commands = realloc(cmdlist.commands, sizeof(Command) * (cmdlist.count + 1));
      cmdlist.commands[cmdlist.count] = parse_command(segment);
      cmdlist.count++;
    }
  }
  free(copy);
  return cmdlist;
}

Command parse_command(char *segment) {
  Command cmd;
  cmd.input_file = NULL;
  cmd.output_file = NULL;
  cmd.args = NULL;
  cmd.argc = 0;
  cmd.name = NULL;

  char *remaining = segment;
  char *tok;

  while ((tok = strsep(&remaining, " \t")) != NULL) {
    if (tok[0] != '\0') {
      if (strcmp(tok, ">") == 0) {
        tok = strsep(&remaining, " \t");
        while (tok != NULL && tok[0] == '\0') {
          tok = strsep(&remaining, " \t");
        }
        if (tok != NULL) {
          cmd.output_file = strdup(tok);
        }
      }
      else if (strcmp(tok, "<") == 0) {
        tok = strsep(&remaining, " \t");
        while (tok != NULL && tok[0] == '\0') {
          tok = strsep(&remaining, " \t");
        }
        if (tok != NULL) {
          cmd.input_file = strdup(tok);
        }
      }
      else {
        cmd.args = realloc(cmd.args, (cmd.argc + 2) * sizeof(char *));
        cmd.args[cmd.argc] = strdup(tok);
        cmd.argc++;
        cmd.args[cmd.argc] = NULL;
      }
    }
  }

  if (cmd.argc > 0) {
    cmd.name = cmd.args[0];
  }

  return cmd;
}

bool is_builtin(const char *name) {
  if (name == NULL) return false;
  return (strcmp(name, "exit") == 0 || strcmp(name, "cd") == 0 || strcmp(name, "path") == 0);
}

void builtin_handler(Command *cmd, int *shell_status, SearchPath *path) {
  if (strcmp(cmd -> name, "exit") == 0) {
    builtin_exit(cmd, shell_status);
  }
  else if (strcmp(cmd -> name, "cd") == 0) {
    builtin_cd(cmd, shell_status);
  }
  else if (strcmp(cmd -> name, "path") == 0) {
    builtin_path(cmd, shell_status, path);
  }
}

void redirect(Command *cmd) {
  if (cmd -> output_file != NULL) {
    int fd = open(cmd -> output_file, O_CREAT | O_TRUNC | O_WRONLY);
    if (fd < 0) {
      perror("open");
      exit(1);
    }
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    close(fd);
  }
  if (cmd -> input_file != NULL) {
    int fd = open(cmd -> input_file, O_RDONLY);
    if (fd < 0) {
      perror("open");
      exit(1);
    }
    dup2(fd, STDIN_FILENO);
    close(fd);
  }
}

int run_command(Command *command, SearchPath *path, int *shell_status) {
  char *fullpath = find_executable(command, path);

  if (fullpath == NULL) {
    fprintf(stderr, "%s: command not found\n", command -> name);
    *shell_status = 1;
    return -1;
  }

  int pid = fork();
  if (pid == 0) {
    redirect(command);
    execv(fullpath, command -> args);
    perror("execv");
    exit(1);
  }

  if (pid < 0) {
    perror("fork");
    *shell_status = 1;
  }
  free(fullpath);
  return pid;
}

void handle_command(Command *command, int *shell_status, SearchPath *path) {
  if (command -> name == NULL) return;
  if (is_builtin(command -> name)) {
    builtin_handler(command, shell_status, path);
  }
  else {
    *shell_status = run_command_wait(command, path);
  }
}

void run_cmdlist(CommandList *cmdlist, int *shell_status, SearchPath *path) {
  if (cmdlist -> delimiter == ';' || cmdlist -> delimiter == '\0') {
    for (int i = 0; i < cmdlist -> count; i++) {
      handle_command(&cmdlist -> commands[i], shell_status, path);
    }
  }
  else if (cmdlist -> delimiter == '&') {
    int *pids = malloc(sizeof(int) * cmdlist -> count);

    for (int i = 0; i < cmdlist -> count; i++) {
      if (cmdlist -> commands[i].name == NULL) {
        pids[i] = -1;
      }
      else {
        pids[i] = run_command(&cmdlist -> commands[i], path, shell_status);
      }
    }

    for (int i = 0; i < cmdlist -> count; i++) {
      if (pids[i] > 0) {
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
          *shell_status = 1;
        }
      }
    }
    free(pids);
  }
}

int run_command_wait(Command *command, SearchPath *path) {
  char *fullpath = find_executable(command, path);

  if (fullpath == NULL) {
    fprintf(stderr, "%s: command not found\n", command -> name);
    return 1;
  }

  int pid = fork();
  if (pid == 0) {
    redirect(command);
    execv(fullpath, command -> args);
    perror("execv");
    exit(1);
  }
  else if (pid < 0) {
    perror("fork");
    free(fullpath);
    return 1;
  }
  else {
    free(fullpath);
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      return WEXITSTATUS(status);
    }
    return 1;
  }
}

char *find_executable(Command *command, SearchPath *path) {
  if (strchr(command -> name, '/')) {
    if (access(command -> name, X_OK) == 0) {
      return strdup(command -> name);
    }
    return NULL;
  }

  char buf[1024];
  for (int i = 0; i < path -> count; i++) {
    snprintf(buf, sizeof(buf), "%s/%s", path -> dirs[i], command -> name);
    if (access(buf, X_OK) == 0) {
      return strdup(buf);
    }
  }
  return NULL;
}

void free_command(Command *cmd) {
  for (int i = 0; i < cmd -> argc; i++) {
    free(cmd -> args[i]);
  }
  free(cmd -> args);
  free(cmd -> input_file);
  free(cmd -> output_file);
}

void free_cmdlist(CommandList *cmdlist) {
  for (int i = 0; i < cmdlist -> count; i++) {
    free_command(&cmdlist -> commands[i]);
  }
  free(cmdlist -> commands);
}