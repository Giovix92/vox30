#include ../../../../Rules.mak
#CROSS_PREFIX = $(CROSS)

#CC = $(CROSS_PREFIX)gcc
#LD = $(CROSS_PREFIX)ld
#AR = $(CROSS_PREFIX)ar
#STRIP = $(CROSS_PREFIX)strip
#RANLIB = $(CROSS_PREFIX)ranlib

LIBICONV_PATH := $(PUBLIC_APPSPATH)/libs/libiconv/libiconv.1.8

CFLAGS := -DHAVE_CONFIG_H -I. -I..  -I../include/ntfs -I$(LIBICONV_PATH)/include/ \
	 -DLTVERSION_LIBNTFS=\"10:0:0\" -O2 -Wall -Werror\
	 -fPIC -DPIC
ifeq ($(CONFIG_SUPPORT_BUILD_DEBUG), 1)
CFLAGS += -g 
endif
ifeq ($(HAVE_SC_CACHE), y)
CFLAGS += -DSC_CACHE
endif

# for definition of iocharset, you can refer to libiconv.
#CFLAGS += -DHAVE_USE_ICONV -DSC_LOCALE_IOCHARSET=\"ISO-8859-1\"
CFLAGS += -DHAVE_USE_ICONV -DSC_LOCALE_IOCHARSET=\"UTF-8\"
CFLAGS += -DSC_FASTMNT #-DDEBUG

LDFLAGS  := -shared -L$(LIBICONV_PATH)/lib -liconv

SRC  := attrib.c  collate.c  debug.c  lcnalloc.c  misc.c \
	unistr.c  attrlist.c  compat.c  device.c  mst.c  \
	bitmap.c  compress.c  device_io.c  index.c  logging.c \
	logfile.c  runlist.c  version.c  bootsect.c  crypto.c  \
	dir.c  inode.c  mft.c  security.c  volume.c

OBJS := $(patsubst %.c,%.o,$(SRC))

LIBNTFS := libntfs.so.10.0.0 libntfs.so.10 libntfs.so
LIBNTFS := $(LIBNTFS)

all: $(LIBNTFS)

libntfs.so.10.0.0: $(OBJS)
	$(CC) $(LDFLAGS) -Wl,-soname -Wl,libntfs.so.10 $(OBJS) -o $@
	$(STRIP) $@
	
libntfs.so.10 libntfs.so: libntfs.so.10.0.0
	ln -sf $< $@

install:
	-cp -a $(LIBNTFS) $(INSTALLDIR)/lib/
#	-cp -a $(LIBNTFS) $(TARGET_DIR)/lib/
clean:
	rm -f *.o *.d  $(LIBNTFS)

%.d: %.c
	@$(CC) $(CFLAGS) -MM $< > $@
	@sed -i "1s/^$*.o:/$*.o $*.d: Makefile /g;" $@

-include $(SRC:.c=.d)
