SUBDIR = openvpn-2.4.6
all:
	cd $(SUBDIR) && test -f Makefile || ./configure CC=$(CC) --host=arm-linux CFLAGS="$(CFLAGS) -I$(PUBLIC_APPSPATH)/libs/lzo/lzo-2.06/include -I$(PUBLIC_APPSPATH)/libs/libpam/libpam-1.3.1/libpam/include"  OPENSSL_LIBS='-L$(PUBLIC_APPSPATH)/libs/openssl/openssl-1.1.0g -lcrypto -lssl' OPENSSL_CFLAGS=-I$(PUBLIC_APPSPATH)/libs/openssl/openssl-1.1.0g/include LZO_LIBS='-L$(PUBLIC_APPSPATH)/libs/lzo -llzo2' LZO_CFLAGS=-I$(PUBLIC_APPSPATH)/libs/lzo/lzo-2.06/include LIBPAM_LIBS='-L$(PUBLIC_APPSPATH)/libs/libpam/libpam-1.3.1/libpam/.libs -lpam' LIBPAM_CFLAGS=-I$(PUBLIC_APPSPATH)/libs/libpam/libpam-1.3.1/libpam/include#--enable-iproute2=yes --with-ssl-headers=$(PUBLIC_APPSPATH)/libs/openssl/openssl-1.1.0g/include/openssl --with-ssl-lib=$(PUBLIC_APPSPATH)/libs/openssl/openssl-1.1.0g --with-lzo-headers=$(PUBLIC_APPSPATH)/libs/lzo/lzo-2.06/include --with-lzo-lib=$(PUBLIC_APPSPATH)/libs/lzo 
	cd $(SUBDIR) && make

install:
	cd $(SUBDIR) && make install


clean:
	cd $(SUBDIR) && make clean



