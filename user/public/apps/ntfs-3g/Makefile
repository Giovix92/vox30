ifeq ($(BRCM_416), 1)
    VERSION_DIR=ntfs-3g-2016.2.22
else
ifeq ($(BRCM_502), 1)
    VERSION_DIR=ntfs-3g-2016.2.22
else
    VERSION_DIR=ntfs-3g
endif
endif

all:
	if test ! -f ./$(VERSION_DIR)/Makefile; then \
		cd ./$(VERSION_DIR); \
		chmod 755 ./configure;\
		./configure CC=$(CC) --host=mips-linux-uclibc;\
	fi
	cd ./$(VERSION_DIR) && make;

ifneq ($(BRCM_416)_$(BRCM_502), 0_0)
install:
	$(STRIP) ./$(VERSION_DIR)/src/.libs/ntfs-3g
	install -m 755 ./$(VERSION_DIR)/src/.libs/ntfs-3g $(INSTALLDIR)/bin/
	install -m 755 ./$(VERSION_DIR)/ntfsprogs/.libs/ntfslabel $(INSTALLDIR)/usr/sbin/
	install -m 755 ./$(VERSION_DIR)/libntfs-3g/.libs/libntfs-3g.so.87.0.0 $(INSTALLDIR)/lib/
	#ln -sf libntfs-3g.so.83 libntfs-3g.so.83.0.0
	ln -sf libntfs-3g.so.87.0.0 libntfs-3g.so.87
	mv libntfs-3g.so.87 $(INSTALLDIR)/lib/
else
install:
	$(STRIP) ./$(VERSION_DIR)/src/.libs/ntfs-3g
	install -m 755 ./$(VERSION_DIR)/src/.libs/ntfs-3g $(INSTALLDIR)/bin/
	install -m 755 ./$(VERSION_DIR)/ntfsprogs/.libs/ntfslabel $(INSTALLDIR)/usr/sbin/
	install -m 755 ./$(VERSION_DIR)/libntfs-3g/.libs/libntfs-3g.so.83.0.0 $(INSTALLDIR)/lib/
	#ln -sf libntfs-3g.so.83 libntfs-3g.so.83.0.0
	ln -sf libntfs-3g.so.83.0.0 libntfs-3g.so.83
	mv libntfs-3g.so.83 $(INSTALLDIR)/lib/
endif

clean:
	-cd ./$(VERSION_DIR) && test -f Makefile &&  make clean && rm -rf Makefile
