CC=gcc
CFLAGS=-O0 -g -Wall -Werror -ansi -pedantic \
 -Wno-implicit-function-declaration -Wno-unused-function

SRC=parse.c scan.c
OBJS=$(SRC:.c=.o)

all: $(OBJS)

clean:
	rm -f $(OBJS)
