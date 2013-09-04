CC=gcc
EXCFLAGS=
CFLAGS=-O0 -g -Wall -Werror -ansi -pedantic \
 -Wno-unused-function

SRC=\
	builtin-stages.exc exec.exc main.exc node.exc parse.exc scan.exc \
	spec.exc transform.exc unparse.exc whereami.exc
CSRC=$(SRC:.exc=.c)
OBJS=$(SRC:.exc=.o)

all: exc

include deps

exc: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o exc

%.c: %.exc
	./exc $(EXCFLAGS) $(CFLAGS) -eonly $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f exc $(OBJS) deps

deps: *.c *.h
	-$(CC) -MM -MP *.c > deps
