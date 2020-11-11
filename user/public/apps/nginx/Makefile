SUBDIR = nginx-1.14.2
all:
	cd $(SUBDIR) && ./configure --prefix=/usr/local/nginx --sbin-path=/usr/local/nginx/nginx --conf-path=/usr/local/nginx --pid-path=/usr/local/nginx/nginx.pid --with-http_ssl_module  --with-cc=$(SYSROOTS)/arm-linux-gcc --with-pcre=$(PUBLIC_APPSPATH)/libs/pcre/pcre-8.42 --with-zlib=$(PUBLIC_APPSPATH)/libs/zlib/zlib-1.2.11 --with-openssl=$(PUBLIC_APPSPATH)/libs/openssl/openssl-1.0.2q 
	cd $(SUBDIR) && make

install:
	cd $(SUBDIR) && make install


clean:
	cd $(SUBDIR) && make clean



