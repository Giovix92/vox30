
CFLAGS = 
all:
	find . |xargs touch
	cd ./libusb-0.1.12 && test -f Makefile || ./configure CC=$(CC) --host=mips-linux-gnu --prefix=$(INSTALLDIR) CXX=$(CROSS_COMPILE)g++ --with-pic
	cd ./libusb-0.1.12 && test -f Makefile && make 
install:
	${STRIP} ./libusb-0.1.12/.libs/libusb-0.1.so.4
	install -D ./libusb-0.1.12/.libs/libusb-0.1.so.4 ${INSTALLDIR}/lib
clean:
	cd ./libusb-0.1.12 && make clean
