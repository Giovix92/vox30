SUBDIR=libseccomp-2.1.1

all:
	cd $(SUBDIR); make

install:
	install $(SUBDIR)/src/libseccomp.so.2.1.1 $(INSTALLDIR)/lib/
	ln -s libseccomp.so.2.1.1 $(INSTALLDIR)/lib/libseccomp.so.2
	ln -s libseccomp.so.2 $(INSTALLDIR)/lib/libseccomp.so
clean:
	cd $(SUBDIR); make clean
