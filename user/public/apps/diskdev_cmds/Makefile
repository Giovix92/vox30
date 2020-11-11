VERSION_DIR=diskdev_cmds-332.14

all:
	cd ./$(VERSION_DIR) && make -f Makefile.lnx 

install:
	install -m 755 ./$(VERSION_DIR)/fsck_hfs.tproj/fsck_hfs $(INSTALLDIR)/bin/fsck.hfsplus

clean:
	cd ./$(VERSION_DIR) && make -f Makefile.lnx clean

