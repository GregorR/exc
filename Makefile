EXC=./exc

SRC=\
	builtin-stages.exc exec.exc main.exc node.exc parse.exc scan.exc \
	spec.exc transform.exc unparse.exc whereami.exc

all: exc

include deps
include Makefile.common

exc: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o exc $(LIBS)

clean:
	rm -f exc $(OBJS) o/exists deps
	-rmdir o

deps: c/*.c c/*.h *.h
	-$(CC) -I. -MM -MP c/*.c > deps
