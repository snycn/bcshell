[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/tthJZgIJ)
# Operating Systems Project 1: Command Shell

## Overview

In this project, you will create the `bcshell`, a command shell that
is capable of launching new processes and performing input/output
redirection.

### Objectives

- Gain exposure to the Unix process API
- Work with low level Unix constructs such as file descriptors
- Gain greater appreciation for modern command shells
- Build a small, but complex, C program

## Background

This project is meant as a conclusion to Unit 1 and 2, connecting our
abstract understanding of the process with a concrete example of
process interaction in the Unix operating system. Chapter 5 of OSTEP
provides an overview of the process API. The process API is not
covered in K&R --- these functions are part of the POSIX standard, but
specific to Unix systems and not part of the broader C standard.

The first Unix command shell was written by Ken Thompson in 1971,
building upon existing traditions of command line interpreters. This
shell was succeeded by the Bourne shell, written in 1976. The POSIX
standard continues to require that `/bin/sh` provide a
Bourne-compatible shell.

Many subsequent shells have gained popularity, such as `csh`, `ksh`,
`tcsh`, `bash`, and `zsh`. `bash`, a GNU clone and expansion of the
Bourne shell, is the default shell on most Linux distributions, while
MacOS switched to `zsh` somewhere in the last decade. You can still
install most, or all, of these shells, and experience the differences
between them.

Until the advent of graphical desktop interfaces in the '90s, the
shell was the primary means of interacting with a computer and its
operating system. Shells became increasingly sophisticated, adding
support for scripting, command completion, and other ease-of-use
features. Shell scripting remains a powerful tool for computer
automation, and many users continue to use the command shell for
day-to-day tasks. Check out the manpage for your shell of choice to
learn more!

## Requirements

You will create a program, `bshell`, that launches into an interactive
command shell. This shell will implement a subset of traditional
command shell functionality, allowing the user to execute commands
with arguments, to run several commands in sequence or in parallel,
and to redirect input and output of commands. For the ease of testing,
your shell will also implement a *batch mode* that allows commands to
be read and executed from a file.

Your program should match the desired behavior **precisely**, as it
will be tested by comparing the actual output to an expected output.
You should also take care to handle various erros and failure states
correctly, as specified below. Some, but not all, tests are provided
to assist you.

### Grading Critera

Your program will be graded on their correctness and the quality of
their implementation.

Correctness will be verified using an automated testing suite, so it
is important that you adhere to any specifications given. Correctness
includes your program's behavior under various edge cases and failure
states. Proper error handling is essential for any system program.

Quality of implementation is a somewhat more subjective metric.
Programs should be well structured, well written, and well formatted.
Your implementation should make reasonably good use of computer
resources.

As this is a project, somewhat more attention will be paid to the
implementation. This presents a greater opportunity for partial credit
when tests fail, but also means that greater scrutiny, and weight,
will be given to issues of code quality. Meeting the requirements
should be your first goal, but improving the clarity, structure, and
performance of your working program will be time well spent.

### Deliverables

You must provide the necessary source files to build `bin/bcshell`.
Your code should build correctly with the command `make all` or `make
bcshell` from the root level of the repository (where this README is
located.)

`Makefile`

The provided Makefile assumes a single source, `src/bcshell.c`. If you
decide to break your code into multiple source files, be sure that
they are correctly added to the Makefile.

`bin/bcshell`

#### Command line invocation

```
Usage: bcshell [BATCHFILE]
```

- When no arguments are provided, `bcshell` should launch an
  interactive shell.
- When `BATCHFILE` is provided, `bcshell` should read commands from
  `BATCHFILE` instead of standard input.
- Invocation with more than one argument should print a usage message
  to stderr and exit with a status code of 1.
- Any errors in opening `BATCHFILE` should print an appropriate error
  message to stderr and exit with a status code of 1.

#### Reading commands

- Your shell should continue to process new commands until it reaches
  `EOF` or receives an `exit` command (see [Builtin
  commands](#builtin-commands)).
- In interactive mode, your shell should print `bcshell> ` before every
  prompt. Be sure to include the space after `>`.
- No prompt should appear in batch mode.

#### Builtin commands

In addition to executing programs, your shell must support the
following builtin commands.

- `exit`

  Exit the shell with the current shell status (See [Exiting the
  shell](#exiting-the-shell)). Produce an error when any arguments are
  provided.

- `cd DIR`

  Change the current directory to `DIR`. Produce an error if changing
  the directory to `DIR` fails, or if more than one argument is
  provided.

  **EXTRA CREDIT:** When no argument is provided, change to the current
  user's **home** directory. (e.g. `/home/francis`).

- `path [-e] [DIR ...]`

  Set the **executable search path** (see [Command
  execution](#command-execution)) to one or more `DIR`. Calling the
  command with no arguments should print the current value.

  When you launch your shell, the only directory in the path should be
  `/bin`.

  **EXTRA CREDIT**: Use the `-e` flag to extend the current path,
  rather than replacing it. Any unknown flags should produce an error.

#### Command execution

Anything entry that is not a builtin command should be treated as an
attempt to run an executable program.

- **You may only use `execv` from the `exec` family of functions to launch executables.**
- If the command name is not part of a path (e.g. `ls` and not
  `/bin/ls`), the shell should search for a matching executable in the
  directories on the **executable search path**.

  When you launch the shell, the only directory on the path should be
  `/bin`. See [Builtin-commands](#builtin-commands) for details on
  modifying the search path.
- If the command name contains a '/', treat it as a relative or
  absolute path; you do not need to search the path.
- All subsequent space-delimited arguments should be passed to `execv`
  as arguments to the command.
- You should be prepared for a large number of arguments, without
  needlessly wasting memory.
- The characters `;`, `>`, `<`, and `&` are reserved. See
  [Redirection](#redirection) and [Command lists](#command-lists).
- Failure to find a matching executable should produce an appropriate
  error and set the current status to a non-zero value (see [Exiting
  the shell](#exiting-the-shell)).
- Failure to launch the program via `execv` should produce an
  appropriate error and set the current status to a non-zero value.
- The current shell status should be set to the status code of an
  executed command once it exits.

#### Redirection

The reserved characters `>` and `<` allow redirecting standard output
and standard input to and from files.

- `> FILE` should redirect standard output **and** standard error to
  `FILE`. (This is slightly different from traditional shell
  behavior.)
- `< FILE` should redirect standard input from `FILE`.
- `> FILE` should **truncate** (i.e. overwrite) `FILE` if it already
  exists.
- Any error in redirection should cancel program execution, produce an
  appropriate error message, and set the shell status to a non-zero
  value (see [Exiting the shell](#exiting-the-shell)).
- This feature is only necessary for execution of programs, not
  builtin commands.

#### Command lists

Using the reserved `;` and `&` characters, it is possible to execute
multiple commands with a single line of input.

- Commands separated by `;` should execute **in sequence**. The first
  command should complete before executing the second command.

  The shell status should become the exit status of the last command
  (see [Exiting the shell](#exiting-the-shell)).
- Commands separated by `&` should execute **in parallel**.

  Each command should be executed immediately, before waiting for any
  preceding commands to complete. The shell should then wait for
  **all** commands to complete before prompting for the next line of
  input.

  The shell status should be non-zero if any of the executed commands
  return a non-zero status code.
- Arguments and redirection for command lists should be handled as
  usual.
- Command lists need not support builtin commands.

#### Errors

You should test every system or library call for errors and handle
those errors appropriately. All error messages generated by the shell
should be printed to standard error.

- Errors in parsing the command line arguments or reading a batch
  input file should cause the program to exit with a non-zero status
  code.
- Once the shell has successfully launched, it should recover
  gracefully from all other errors and continue prompting the user for
  additional input.
- Errors parsing an input line should cancel execution of any commands
  executed on that line.

#### Exiting the shell

- Once the shell has successfully launched, it should only exit after
  encountering `EOF` or the `exit` builtin command.
- The exit status of the shell should be the status code of the last
  run command. This does not include the `exit` command.

## Guide

### Getting started

Try breaking your program into small, verifiable chunks of
functionality. Each piece of functionality should touch several
components, allowing you to slowly build out your design in a
verifiable way. As you progress, your program should pass more and
more of the test suite.

Launching into the interactive shell might be a good first feature.
Then the `exit` builtin might be nice so that you can quit gracefully.
Getting batch input working will make running your own tests easier.

Try to commit your work to the `git` repository after every
successfully implemented feature. This makes it easier to abandon
failed experiments and get back to a working state, without fear of
losing work.

After a piece of functionality is complete, it is often helpful to
look back over your code and **refactor** it --- improving the
structure and clarity to prepare for your next step.

### Getting unstuck

This is a large program with many components. Expect to spend more
time than you have on the homework assignments. Be sure to make the
most of your available resources -- read the relevant sections in the
OSTEP textbook. Review sections on working with strings and files in
K&R. Read the manpages for each function you are using, and spend time
trying to decipher any examples that they provide.

Experiment with your own working shell to better understand the
expected behavior, though take note of places where the requirements
for this project might diverge from those of a fully-featured shell.

Look closely at tests you are failing to help you clarify requirements
and find clues in potential problems. The tests are there for your
benefit.

If you are completely stuck, push the code that you have to GitHub and
send me an e-mail with your challenges. I will try to take a look and
offer some hints to get you moving.

If you are unable to unable to complete the project, prefer an
incomplete, but properly functioning implementation over a broad
implementation that provides incorrect versions of all request
features.

### Capturing shell input

You will probably want to use buffered IO for reading input to the
shell. See homework 3 and the manpages for me details on working with
`fopen`, `fclose`, etc. The `getline` function might also be
particularly useful for dealing with input lines of arbitrary length.

### Parsing shell input

Parsing strings can be a tricky problem, and there are several valid
approaches to this program. You might want to explore the `strtok` and
`strsep` functions; the latter, `strsep`, is somewhat simpler to use
and preferable for this assignment, though it derives from BSD Unix
and is not present in all C library implementations. The `strchr` is
an alternative for a slightly more hands-on approach.

Arguments are any two tokens delimited by any amount of whitespace, so
be prepared for two or more spaces, as well as tabs. Reserved
characters such as `>` and `&`, however, might not be separated by
spaces. Be careful about assumptions you make about the nature of the
input.

Look at the section on [Executing programs](#executing-programs) for
details on arranging your arguments into an **argument vector**.

### Forking processes

Caveats to `fork` notwisthstanding, we will be using `fork` to make
new subprocesses that will host programs launched by our shell. You
should use this opportunity to learn about `fork` as a traditional
Unix programming construct, but be aware that there may be
higher-level and more appropriate alternatives in the future.

Read the manpages for `fork`, `wait`, and `waitpid`. After running
`fork`, you will have two branched execution contexts. One, the
original parent process, will receive a non-zero integer as a return
value. This is the `pid` of the child process, and can be used with
the `waitpid` function. The other process, will receive a return call
of 0. This will be the child process, and it should `exec` into the
new program, after completing some initial setup.

The parent process will need to use `wait` or `waitpid` to capture the
exit status of the child process.

### Searching for executables

Unless an absolute path to the executable is provided, you will need
to search the `path` variable to find the location of a valid
executable matching the command name. To test each possible directory,
consider using the `access` system call. By concatenating the
candidate path and the command name into a single str, and calling
`access(fullpath, X_OK)`, you can check to see if an executable file
is located at `fullpath`.

Alternatively, you can use the `open` system call to open a directory,
and then use the `faccessat` system call to test a path relative to an
open file descriptor. Be sure to close any file descriptors once you
are done with them.

### Redirecting standard IO

A forked process is created with a copy of its parent processes's file
descriptors, which point to the same global file table within the
kernel. In order to **redirect** IO, we need to change the file
descriptor table **after** forking from the parent process (leaving
parent file descriptors unchanged), but before executing the new
program, when we no longer have any control over the actions of the
child process.

When using the `open` system call to create a new entry in the file
descriptor table, the OS will assign us the **lowest available** file
descriptor. Chapter 5 in OSTEP contains an example of using `close` on
an existing standard IO descriptor, so that the subsequent call to
`open` will replace that descriptor with the newly opened file.

Read the manpage for `open` and be sure you are opening files for
reading (standard input) or writing (standard output) appropriately.

### Executing programs

The `exec` system call will replace the currently running program in a
process with another program, read from the filesystem. The POSIX
standard provides a family of `exec` functions, each providing
slightly different behavior on top of the kernel interface. (In Linux,
this is the `execve` system call.) We will be using the `execv` function.

The `execv` function accepts two arguments -- the first, `const char
*pathname`, is the path to the file being executed. The second, `char
*const argv[]` is the **vector** of arguments to be passed to the new
program. (This is the same `argv` that is passed to `main`.) A
**vector** is an array of pointers (in this case, `const char *`),
terminated by a `NULL` pointer. The first entry to `argv`, by
convention, is the filename used to call the program.

You will need to construct this vector; keep in mind that there could
be an arbitrary number of arguments.

### Breaking your program into multiple source files

You may find it helpful to break your component into separate source
files to help structure your program. On large programs, this also
reduces the incremental build times for your project. When working
with multiple sources, keep the following things in mind:

#### Object files, or compilation units

Each `.c` source file is compiled into a single `.o` **object file**.
In order to compile, your source must include a declaration of every
**symbol** (e.g. function name, variable), but need not include a
**definition** of every symbol. This allows the following object file
to compile, whether or not the definitions for `bcgetchar` and
`bcputchar` exist.

```
int bcgetchar();
int bcputchar(char c);

int main() {
  bcputchar('a');
  bcgetchar();
}
```

Declarations of this sort are usually placed in **header files** (e.g.
`stdio.h`), so that they can be easily included into other source
files. Header files will also often contain struct and type
definitions, macro definitions, and declarations of external
variables. Header files may even contain compileable function
definitions, though this should be done with care. The `#include`
compiler directive actually **copies** the contents of the header file
directly into compilation unit, as if you had pasted the file into
your source file.

An object file is created with the `-c` option to `gcc`, e.g. `gcc -c
bcshell.c -o bcshell.o`.

#### Linking executables

In order to create an executable file, all of the symbolic references
in your object files must be **linked** together and replaced with the
memory address that your processor will use for various jump, load,
and store instructions. At this stage, the **linker** will look for an
actual definition of every symbol, either in your object files or in a
provided **library file**. (Dynamically linked libraries will be
checked against symbols, but references will still be left in a
symbolic state, to be finalized whenever the executable is loaded.)

The linker can be run as its own program, but can also be run
indirectly using the `gcc` command. Just list all of the object files
you would like to link into your final executable on the command line,
e.g. `gcc bcshell.o parser.o -o bin/bcshell`. Standard libraries are
linked in by default, while other libraries must be linked in using
additional command line options.

Linking is much faster than compiling, so it is common to only rebuild
source files that have changed, and then quickly link everything
together into a single executable file.

#### Managing dependencies in `make`

For `make` build your executable from multiple object files, you need
to add additional **dependencies** to that executable's **rule** in
your `Makefile`.

```
# build bcshell from bcshell.o and parser.o
bin/bcshell: src/bcshell.o src/parser.o

# recompile bcshell.o if bcshell.c or parser.h changes
src/bcshell.o: src/bcshell.c src/parser.h

# recompile parser.o if parser.c or parser.h changes
src/parser.o: src/parser.c src/parser.h
```

Dependency management becomes a major challenge for large software
projects. Build managers like [CMake](https://cmake.org) and
[Bazel](https://bazel.build) provide higher level tools for defining
and building dependencies that can ensure consistency.


## Submission

Your code should be submitted using GitHub classroom, conforming to
the directory structure of the original starter repository. The
Makefile must build successfully and all tests must pass on a Linux
system in order to receive full credit. See [Grading
Criteria](#grading-criteria) for more details on grading.

