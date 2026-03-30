#ifndef BUILTINS_H
#define BUILTINS_H

#include "structs.h"

void builtin_exit(Command *cmd, int *shell_status);
void builtin_cd(Command *cmd, int *shell_status);
void builtin_path(Command *cmd, int *shell_status, SearchPath *path);

#endif
