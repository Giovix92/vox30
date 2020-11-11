
CUR_DIR = $(shell pwd)
TARGET_HOME = $(shell pwd)/../../../..
CFLAGS = -L$(INSTALLDIR)/lib/
CPPFLAGS = -I$(PUBLIC_APPSPATH)/libs/zlib/zlib-1.2.11/

all:
	test -f Makefile || ./configure --host=mips64-target-linux-gnu  --enable-shared=no --enable-static=yes CC=$(CROSS_COMPILE)gcc CXX=$(CROSS_COMPILE)g++ AR=$(CROSS_COMPILE)ar LD=$(CROSS_COMPILE)ld RANLIB=$(CROSS_COMPILE)ranlib STRIP=$(CROSS_COMPILE)strip CPPFLAGS=$(CPPFLAGS)
	test -f Makefile && make

install:
	cp -a ./libtiff/.libs/libtiff.so* ../lib/

clean:
	-make distclean

.PHONY: all install clean
