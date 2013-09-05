CC=gcc
EXCFLAGS=-ec-prefix c/
#CFLAGS=-O0 -g -Wall -Werror -ansi -pedantic -I. \
# -Wno-unused-function
CFLAGS=-O0 -g -I.

# FIXME: These are GNU make specific :(
SRC=\
	builtin-stages.exc exec.exc main.exc node.exc parse.exc scan.exc \
	spec.exc transform.exc unparse.exc whereami.exc
CSRC=$(addprefix c/,$(SRC:.exc=.c))
OBJS=$(addprefix o/,$(SRC:.exc=.o))

all: exc

include deps

exc: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o exc

csrc: $(CSRC)

c/exists:
	mkdir -p c
	touch c/exists

c/%.c: %.exc c/exists
	-./exc $(EXCFLAGS) $(CFLAGS) -eonly $<

o/exists:
	mkdir -p o
	touch o/exists

o/%.o: c/%.c o/exists $(CSRC)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f exc $(OBJS) o/exists deps
	-rmdir o

deps: c/*.c c/*.h *.h
	-$(CC) -I. -MM -MP c/*.c > deps
