CC=gcc
CFLAGS=-O0 -g -Wall -Werror -ansi -pedantic
SRC=scan.c
OBJS=$(SRC:.c=.o)

all: $(OBJS)

clean:
	rm -f $(OBJS)
