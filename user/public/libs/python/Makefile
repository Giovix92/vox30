SUBDIR=Python-3.5.2

all:
	cd $(SUBDIR); test -f Makefile || ./configure --host=arm-linux --build=arm --disable-ipv6 ac_cv_file__dev_ptmx=no ac_cv_file__dev_ptc=no ac_cv_have_long_long_format=yes
	cd $(SUBDIR); make libpython3.so

install:
	install $(SUBDIR)/libpython3.5m.so $(INSTALLDIR)/lib/
	install $(SUBDIR)/libpython3.so $(INSTALLDIR)/lib/

clean:
	cd $(SUBDIR); make clean
