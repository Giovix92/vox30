
UTILOBJ=utils.o rt_names.o ll_types.o ll_proto.o ll_addr.o inet_proto.o
#options if you have a bind>=4.9.4 libresolv (or, maybe, glibc)
ADDLIB=

#options for decnet
ADDLIB+=dnet_ntop.o dnet_pton.o

#options for ipx
ADDLIB+=ipx_ntop.o ipx_pton.o


CFLAGS += -Os -Wall -s -fPIC 
NLOBJ=ll_map.o libnetlink.o
all: libnetlink.so libutils.so

libnetlink.so: $(NLOBJ)
	$(CC) -Os -s -shared -fPIC $(LDFLAGS) -o $@ -Wl,-soname,$@ $(NLOBJ)

libutils.so: $(UTILOBJ) $(ADDLIB)
	$(CC) -Os -s -shared -fPIC $(LDFLAGS) -o $@ -Wl,-soname,$@ $(UTILOBJ) $(ADDLIB) 

%.o:%.c
	$(CC) $(CFLAGS) -I ../include -c $^

install:
	/usr/bin/install libnetlink.so $(INSTALLDIR)/lib
	/usr/bin/install libutils.so $(INSTALLDIR)/lib

clean:
	rm -f $(NLOBJ) $(UTILOBJ) $(ADDLIB) libnetlink.so libutils.so

