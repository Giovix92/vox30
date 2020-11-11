
export CFLAGS += -D__SC_BUILD__ -I$(PRIVATE_APPSPATH)/include -L$(INSTALLDIR)/lib $(X_PRIVATE_LIBS) $(X_PUBLIC_LIBS) -lm -march=74kc -O2 -fomit-frame-pointer -pipe -fno-strict-aliasing 
all:
	cd ./radvd-1.9 && test -f Makefile || ./configure CC=$(CC) --host=mips-target-linux-gnu --prefix=$(INSTALLDIR)  --enable-static=yes --enable-shared=no
	cd ./radvd-1.9 && make 
install:
	cp ./radvd-1.9/radvd ./radvd-1.9/rsdvd
	$(STRIP) ./radvd-1.9/rsdvd
	install -m 755 ./radvd-1.9/rsdvd $(INSTALLDIR)/usr/sbin/

clean:
	cd ./radvd-1.9 && test -f Makefile &&  make clean && rm rsdvd

