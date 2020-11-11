
TARGET_DIR = ./dmalloc-5.5.2
all:
	cd ./dmalloc-5.5.2 && chmod 755 configure
	cd ./dmalloc-5.5.2 &&  test -f Makefile || (find ./ |xargs touch && ./configure prefix=${CROSS_DIR}/usr/mips-unknown-linux-uclibc/sysroot/usr CC=${CC} CFLAGS=-fPIC --host="mips-linux" --enable-threads --enable-shlib --disable-cxx)
	cd ./dmalloc-5.5.2 &&  test -f Makefile && make 
 

install:
	$(STRIP) $(TARGET_DIR)/dmalloc
	cp $(TARGET_DIR)/dmalloc $(INSTALLDIR)/usr/sbin/
	cp $(TARGET_DIR)/libdmalloc.so $(INSTALLDIR)/usr/lib/
	cp $(TARGET_DIR)/libdmallocth.so $(INSTALLDIR)/usr/lib/

clean:
	cd ./dmalloc-5.5.2 && test -f Makefile && make clean
