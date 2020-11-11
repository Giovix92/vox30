
export CFLAGS += -D__SC_BUILD__ -L$(INSTALLDIR)/lib -I$(PUBLIC_APPSPATH)/libs/zlib/zlib-1.2.11/
export LDFLAGS += -L$(INSTALLDIR)/lib $(X_PRIVATE_LIBS) $(X_PUBLIC_LIBS)

TARGET_DIR = ./privoxy-3.0.26-stable

all:
	touch $(TARGET_DIR)/pcre/chartables.c 
	cd $(TARGET_DIR);test -f GNUmakefile || (autoheader && autoconf && ./configure CC=$(CC) --host=arm-unknown-linux-gnu --prefix=$(INSTALLDIR))
	cd $(TARGET_DIR) && make 
install:
	install -m 755 $(TARGET_DIR)/privoxy $(INSTALLDIR)/usr/sbin/
	cp -rf $(TARGET_DIR)/templates/ $(INSTALLDIR)/etc/
clean:
	cd $(TARGET_DIR) && test -f GNUmakefile && $(MAKE) clean


