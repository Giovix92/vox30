
export CFLAGS += -I$(SYSROOTS)../sysroot/usr/include 
all:
	cd ./lzo-2.06 && test -f Makefile || ./configure CC=$(CC) --host=mips-linux-gnu --prefix=$(INSTALLDIR) --enable-static=yes --enable-shared=yes
	cd ./lzo-2.06 && test -f Makefile && make 
	cp ./lzo-2.06/src/.libs/liblzo2.so* ./
	install -m 0755 liblzo2.so.2.0.0 $(INSTALLDIR)/lib
	ln -sf liblzo2.so.2.0.0 $(INSTALLDIR)/lib/liblzo2.so.2
	ln -sf liblzo2.so.2 $(INSTALLDIR)/lib/liblzo2.so
install:


clean:
	cd ./lzo-2.06 && test -f Makefile &&  make clean
	-rm -f liblzo2.so*


