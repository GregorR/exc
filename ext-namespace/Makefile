ROOT_DIR=..
SRC=exc-namespace.exc
LIB=../exc-namespace.la

all: $(LIB)

include $(ROOT_DIR)/Makefile.common

$(LIB): $(OBJS)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) -module -rpath $(LIB_PREFIX) $(OBJS) -o $@ ../libexc.la

clean:
	rm -rf .libs o/.libs
	rm -f $(LIB) o/*.lo o/*.o o/exists
	rmdir o
