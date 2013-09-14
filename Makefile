ROOT_DIR=.

SRC=\
	builtin-stages.exc exec.exc node.exc parse.exc scan.exc spec.exc \
	transform.exc unparse.exc whereami.exc

all: exc

include deps
include Makefile.common

libexc.la: $(OBJS) $(MLIBTOOL)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) -rpath $(LIB_PREFIX) $(OBJS) -o $@ $(LIBS)

exc: o/main.o libexc.la $(MLIBTOOL)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) o/main.o -o exc libexc.la

clean:
	rm -rf .libs o/.libs
	rm -f exc libexc.la o/*.lo o/*.o o/exists deps
	rm -f mlibtool
	-rmdir o

deps: c/*.c c/*.h *.h
	-$(CC) -I. -MM -MP c/*.c > deps
