
export CFLAGS += -D__SC_BUILD__ -L$(INSTALLDIR)/lib -lslog -O2 -fomit-frame-pointer -pipe -fno-strict-aliasing 
CFLAGS += -I$(PUBLIC_APPSPATH)/include
all:
	cd ./radvd-1.9 && test -f Makefile || ./configure CC=$(CC) --host=mips-target-linux-gnu --prefix=$(INSTALLDIR)  --enable-static=yes --enable-shared=no
	cd ./radvd-1.9 && make 
install:
	$(STRIP) ./radvd-1.9/radvd
	install -m 755 ./radvd-1.9/radvd $(INSTALLDIR)/usr/sbin/

clean:
	cd ./radvd-1.9 && test -f Makefile &&  make clean

