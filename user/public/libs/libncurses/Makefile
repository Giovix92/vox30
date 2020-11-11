SUBDIR=$(PUBLIC_APPSPATH)/libs/libncurses/ncurses-6.1

all:
	cd $(SUBDIR) && test -f Makefile || ./configure  CC=$(CC)  --host=arm-linux  --prefix=$(INSTALLDIR)  --with-shared\
														--without-ada --without-tests --with-build-cflags=-Os 
	cd $(SUBDIR) && test -f Makefile && make 

install:
	@echo install libncurses
	install -m 755 $(SUBDIR)/lib/libncurses.so.6.1  $(INSTALLDIR)/lib/
	ln -sf libncurses.so.6.1  $(INSTALLDIR)/lib/libncurses.so.6
	ln -sf libncurses.so.6.1  $(INSTALLDIR)/lib/libncurses.so

clean:
	-cd $(SUBDIR) && test -f Makefile &&  make clean 
