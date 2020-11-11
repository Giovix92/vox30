
CFLAGS += -Os -s -Wall
CFLAGS += -Werror -I $(PRIVATE_APPSPATH)/include
ifeq ($(CONFIG_SUPPORT_BUILD_DEBUG), 1)
CFLAGS += -g 
endif

BINS = libnv.so #libnvram.so #libcms_msg.so

all: $(BINS)

libnv.so : nvram.o nvram_lock.o
	$(CC) $(CFLAGS) -Os -s -shared -Wl,-soname,libnv.so -o libnv.so $^ 


nvram.o : nvram.c
	$(CC) $(CFLAGS) -Os -s -Wall -Werror -fPIC -c $^ -o $@


nvram_lock.o: nvram_lock.c
	$(CC) $(CFLAGS) -Os -s -Wall -fPIC -c $^ -o $@

install :
	cp -f libnv.so $(INSTALLDIR)/lib/	

clean:
	rm -rf *~ *.o *.so $(BINS)

