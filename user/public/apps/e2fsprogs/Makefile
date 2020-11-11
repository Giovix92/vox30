
export CFLAGS = -I$(SYSROOTS)../sysroot/usr/include 
ifeq ($(BRCM_502), 1)
    CFLAGS += -DBRCM_502
endif
all:
	cd ./e2fsprogs-1.41.14 && test -f Makefile ||  ./configure CC=$(CC) --host=mips-linux-gnu --prefix=$(INSTALLDIR) --enable-static=yes --enable-shared=yes
	cd ./e2fsprogs-1.41.14 && test -f Makefile && touch intl/plural.c  && make 
	install -D ./e2fsprogs-1.41.14/lib/libuuid.a ./
install:


clean:
	cd ./e2fsprogs-1.41.14 && make clean
	-rm -f libuuid.a


