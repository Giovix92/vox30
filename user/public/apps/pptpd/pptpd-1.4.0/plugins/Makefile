CC	= arm-linux-gcc 
COPTS	= -O2 -g
CFLAGS	= $(COPTS) -I.. -I../../include -fPIC
CFLAGS  += -I$(PUBLIC_APPSPATH)/apps/ppp/ppp-2.4.5
LDFLAGS	= -shared
LDADD   = -lutil
INSTALL	= install -o root
prefix  = /usr

PLUGINS = pptpd-logwtmp.so

# include dependencies if present
ifeq (.depend,$(wildcard .depend))
include .depend
endif

all:	$(PLUGINS)

%.so: %.c
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $^ $(LDADD)

LIBDIR	= $(INSTALLDIR)/lib/

install: $(PLUGINS)
	install $(PLUGINS) $(INSTALLDIR)/lib/$(PLUGINS)

uninstall:
	rm -f $(LIBDIR)$(PLUGINS)

clean:
	rm -f *.o *.so *.a

depend:
	$(CPP) -M $(CFLAGS) *.c >.depend
