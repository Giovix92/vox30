SUBDIR=libcap-2.25

all:
	cd $(SUBDIR); make

install:
	$(STRIP) $(SUBDIR)/libcap/libcap.so.2.25
	install -m 755 $(SUBDIR)/libcap/libcap.so.2.25 $(INSTALLDIR)/lib/
	ln -sf libcap.so.2.25 $(INSTALLDIR)/lib/libcap.so.2
	ln -sf libcap.so.2 $(INSTALLDIR)/lib/libcap.so

clean:
	cd $(SUBDIR); make clean
