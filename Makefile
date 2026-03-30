CC      = gcc
CFLAGS  = -g
CFLAGS += -Wall
CFLAGS += -std=c11
CFLAGS += -D_POSIX_C_SOURCE=200809L
CFLAGS += -D_DEFAULT_SOURCE

all: bcshell test

bcshell: bin/bcshell

bin/bcshell: src/bcshell.o src/builtins.o
	$(CC) $(CFLAGS) $^ -o $@

src/bcshell.o: src/bcshell.c src/structs.h src/builtins.h

src/builtins.o: src/builtins.c src/structs.h src/builtins.h

test: bin/bcshell tests/* run-tests.sh
	./run-tests.sh -cv

clean:
	rm -rf src/*.o bin/* tests-out/*

.PHONY: all bcshell clean
