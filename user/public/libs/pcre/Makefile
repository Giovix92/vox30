SUBDIR=pcre-8.42
all:
	cd $(SUBDIR) &&  test -f Makefile || ./configure prefix=${INSTALLDIR} CC=${CC}  --host="arm-linux"
	cd $(SUBDIR); make

install:
	install $(SUBDIR)/.libs/libpcreposix.so.0.0.6 $(INSTALLDIR)/lib/
	ln -s libpcreposix.so.0.0.6 $(INSTALLDIR)/lib/libpcreposix.so.0
	ln -s libpcreposix.so.0 $(INSTALLDIR)/lib/libpcreposix.so
	install $(SUBDIR)/.libs/libpcre.so.1.2.10 $(INSTALLDIR)/lib/
	ln -s libpcre.so.1.2.10 $(INSTALLDIR)/lib/libpcre.so.1
	ln -s libpcre.so.1 $(INSTALLDIR)/lib/libpcre.so
clean:
	cd $(SUBDIR); make clean
