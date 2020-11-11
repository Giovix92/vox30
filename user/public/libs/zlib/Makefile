DIR = zlib-1.2.11
all:
	cd ${DIR} && test -f Makefile || (CC=${CC} && ./configure  --prefix=$(INSTALLDIR))
	cd ${DIR} && make 

install:
	cd ${DIR} && cp -rdf libz.so*  $(INSTALLDIR)/lib

clean:
	cd ${DIR} && make clean
