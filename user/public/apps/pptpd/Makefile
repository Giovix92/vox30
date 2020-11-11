SUBDIR = ./pptpd-1.4.0

all:
	cd $(SUBDIR) && test -f Makefile || ./configure --with-opt-refreshtime=32 CC=$(CC) --host=mips-target-linux-gnu --prefix=/usr  --enable-static=yes --enable-shared=no
	cd $(SUBDIR) && make
install:
	$(STRIP) $(SUBDIR)/pptpd
	cd $(SUBDIR) && make install

clean:
	cd $(SUBDIR) && test -f Makefile &&  make distclean
