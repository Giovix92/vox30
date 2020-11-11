SUBDIR=$(PUBLIC_APPSPATH)/libs/libreadline/readline-5.2
all:
	cd $(SUBDIR) && test -f Makefile || ./configure  --host=arm-linux --prefix=$(INSTALLDIR) --includedir=$(PUBLIC_APPSPATH)/include
	cd $(SUBDIR) && test -f Makefile && make && make install-headers 

install:
	@echo install libreadline
	install -m 755 $(SUBDIR)/shlib/libreadline.so.5.2  $(INSTALLDIR)/lib/
	install -m 755 $(SUBDIR)/shlib/libhistory.so.5.2  $(INSTALLDIR)/lib/
	ln -sf libreadline.so.5.2  $(INSTALLDIR)/lib/libreadline.so.5
	ln -sf libreadline.so.5.2  $(INSTALLDIR)/lib/libreadline.so
	ln -sf libhistory.so.5.2  $(INSTALLDIR)/lib/libhistory.so.5
	ln -sf libhistory.so.5.2  $(INSTALLDIR)/lib/libhistory.so

	

clean:
	-cd $(SUBDIR) && test -f Makefile &&  make clean
