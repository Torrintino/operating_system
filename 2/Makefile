#!/usr/bin/make
.SUFFIXES:
.PHONY: all run pack clean
SRC = shell.c line.c process.c
TAR = shell
OBJ = $(SRC:%.c=%.o)
PCK = lab-2.zip

CFLAGS = -std=gnu11 -c -g -Os -Wall -Werror

%.o: %.c %.h
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

shell: $(OBJ)
	$(CC) -o shell $^

all: $(TAR)

run: $(TAR)
	./shell

pack:
	zip $(PCK) $(SRC) $(wildcard *.h) Makefile

clean:
	$(RM) $(RMFILES) $(OBJ) $(TAR) $(PCK)
