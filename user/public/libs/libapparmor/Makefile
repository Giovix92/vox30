SUBDIR=apparmor-2.12/libraries/libapparmor/

all:
	cd $(SUBDIR); test -f Makefile || (sh ./autogen.sh && sh ./configure --host=arm-linux)
	cd $(SUBDIR); make
	#cd $(SUBDIR); make check

install:
	install $(SUBDIR)src/.libs/libapparmor.so.1.4.2 $(INSTALLDIR)/lib/
	ln -s libapparmor.so.1.4.2 $(INSTALLDIR)/lib/libapparmor.so.1
	ln -s libapparmor.so.1 $(INSTALLDIR)/lib/libapparmor.so
clean:
	cd $(SUBDIR); make clean
