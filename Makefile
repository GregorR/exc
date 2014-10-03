ROOT_DIR=.

SRC=\
	builtin-stages.exc exec.exc node.exc parse.exc scan.exc spec.exc \
	transform.exc unparse.exc whereami.exc

# Note that namespace must come first, as it's used by exc itself
EXTENSIONS=\
	namespace \
	ggggc2

all: exc
	for i in $(EXTENSIONS); do \
	    $(MAKE) -C ext-$$i; \
	done

include deps
include Makefile.common

libexc.la: $(OBJS) $(MLIBTOOL)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) -rpath $(LIB_PREFIX) $(OBJS) -o $@ $(LIBS)

exc: o/main.o libexc.la $(MLIBTOOL)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) o/main.o -o exc libexc.la

install: all
	mkdir -p $(BIN_PREFIX)
	$(LIBTOOL) --mode=install install exc $(BIN_PREFIX)
	mkdir -p $(LIB_PREFIX)
	$(LIBTOOL) --mode=install install libexc.la $(LIB_PREFIX)
	mkdir -p $(EXT_PREFIX)
	for i in $(EXTENSIONS); do \
	    $(LIBTOOL) --mode=install install exc-$$i.la $(EXT_PREFIX); \
	done
	mkdir -p $(SPEC_PREFIX)
	for i in spec/*.excspec; do \
	    install -m0644 $$i $(SPEC_PREFIX); \
	done

clean:
	rm -rf .libs o/.libs
	rm -f exc libexc.la o/*.lo o/*.o o/exists deps
	rm -f mlibtool
	-rmdir o
	for i in $(EXTENSIONS); do \
	    $(MAKE) -C ext-$$i clean; \
	done

deps: c/*.c c/*.h src/*.h
	-$(CC) -Isrc -MM -MP c/*.c > deps
