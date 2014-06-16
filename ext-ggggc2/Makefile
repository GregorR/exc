ROOT_DIR=..
SRC=exc-ggggc2.exc

all: ../exc-ggggc2.la

include ../Makefile.common

../exc-ggggc2.la: $(OBJS)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) -module -rpath $(LIB_PREFIX) $(OBJS) -o $@ ../libexc.la

clean:
	rm -rf c o .libs ../exc-ggggc2.la
