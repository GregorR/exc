CC=gcc
CFLAGS=-O0 -g -Wall -Werror -ansi -pedantic \
 -Wno-unused-function

SRC=\
	builtin-stages.c exec.c main.c node.c parse.c scan.c spec.c \
	transform.c unparse.c whereami.c
OBJS=$(SRC:.c=.o)

all: exc

include deps

exc: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o exc

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f exc $(OBJS) deps

deps: *.c *.h
	-$(CC) -MM -MP *.c > deps
