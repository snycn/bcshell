#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct {
  char *name;
  char **args;
  int argc;
  char *input_file;
  char *output_file;
} Command;

typedef struct {
  Command *commands;
  int count;
  char delimiter;
} CommandList;

typedef struct {
  char **dirs;
  int count;
} SearchPath;

#endif