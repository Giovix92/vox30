
CUR_PATH = $(shell pwd)

CFLAGS += -Os -Wall -s -fPIC -I./ -I./include

LIBUBI_TARGET = libubicommon.so libubicommon.a

LIBUBI_OBJS += libubiformat.o  
LIBUBI_OBJS += libcrc32.o
LIBUBI_OBJS += libfec.o
LIBUBI_OBJS += libiniparser.o
LIBUBI_OBJS += libmtd.o
LIBUBI_OBJS += libmtd_legacy.o
LIBUBI_OBJS += libscan.o
LIBUBI_OBJS += libubi.o
LIBUBI_OBJS += libubigen.o
LIBUBI_OBJS += ubiutils-common.o
LIBUBI_OBJS += dictionary.o

all: $(LIBUBI_TARGET)

libubicommon.so : $(LIBUBI_OBJS)
	$(CC) -Os -s -shared -fPIC $(LDFLAGS) -o $@ -Wl,-soname,$@ $(LIBUBI_OBJS)

libubicommon.a : $(LIBUBI_OBJS)
	$(AR) cr $@ $(LIBUBI_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
		
install :
	cp -f  libubicommon.so $(INSTALLDIR)/lib
	
clean:
	rm -rf $(LIBUBI_TARGET) $(LIBUBI_OBJS)

