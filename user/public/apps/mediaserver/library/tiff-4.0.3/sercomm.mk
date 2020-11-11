CUR_DIR = $(shell pwd)
TARGET_HOME = $(shell pwd)/../../../..


all:
	test -f Makefile || ./configure --host=mips-target-linux-gnu  --enable-shared=no --enable-static=yes --with-zlib-include-dir=$(PUBLIC_APPSPATH)/libs/zlib/zlib-1.2.11/ --with-zlib-lib-dir=$(INSTALLDIR)/lib/ --with-jpeg-include-dir=$(CUR_DIR)/../jpeg-6b --with-jpeg-lib-dir=$(CUR_DIR)/../jpeg-6b/.libs CC=$(CROSS_COMPILE)gcc CXX=$(CROSS_COMPILE)g++ AR=$(CROSS_COMPILE)ar LD=$(CROSS_COMPILE)ld RANLIB=$(CROSS_COMPILE)ranlib STRIP=$(CROSS_COMPILE)strip
	test -f Makefile && make

install:
	cp -a ./libtiff/.libs/libtiff.so* ../lib/

clean:
	-make distclean

.PHONY: all install clean
