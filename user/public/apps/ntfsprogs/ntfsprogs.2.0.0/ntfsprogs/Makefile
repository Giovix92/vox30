#include ../../../../Rules.mak
#CROSS_PREFIX = $(CROSS)
TARGET_DIR=$(INSTALLDIR)

#CC = $(CROSS_PREFIX)gcc
#LD = $(CROSS_PREFIX)ld
#STRIP = $(CROSS_PREFIX)strip

FUSE_PATH := $(PUBLIC_APPSPATH)/libs/fuse.2.7.4
LIBICONV_PATH := $(PUBLIC_APPSPATH)/libs/libiconv/libiconv.1.8

CFLAGS  :=  -DHAVE_CONFIG_H -I. -I..  -I../include/ntfs   \
	-D_FILE_OFFSET_BITS=64 -I$(FUSE_PATH)/include  \
	-DFUSE_USE_VERSION=26 -g -O2 -Wall -Werror

LDFLAGS := -lpthread -lrt -ldl -L$(FUSE_PATH)/lib -lfuse -L$(LIBICONV_PATH)/lib -liconv

SRC  := utils.c ntfsmount.c

ifeq ($(HAVE_SC_CACHE), y)
CFLAGS += -DSC_CACHE
SRC += sc_cache.c
endif
# fast mount 
CFLAGS += -DSC_FASTMNT

OBJS := $(patsubst %.c,%.o,$(SRC))

BIN  := ntfsmount ntfslabel

all: $(BIN)

ntfsmount: utils.o ntfsmount.o sc_cache.o ../libntfs/libntfs.so
	$(CC) $(LDFLAGS) -o $@ $^
	$(STRIP) $@

ntfslabel: 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ ntfslabel.c utils.c ../libntfs/libntfs.so
	$(STRIP) $@
	
ntfsinfo: 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@  ntfsinfo.c utils.c ../libntfs/libntfs.so
	$(STRIP) $@
	
ntfsls: 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@  ntfsls.c utils.c ../libntfs/libntfs.so
	$(STRIP) $@
	
install:
	-cp -a $(BIN) $(TARGET_DIR)/usr/sbin/
	ln -sf ../bin/ntfsmount $(TARGET_DIR)/sbin/mount.ntfs-fuse 
	
clean:
	rm -f *.o $(BIN) *.d

%.d: %.c
	@$(CC) $(CFLAGS) -MM $< > $@
	@sed -i "1s/^$*.o:/$*.o $*.d: Makefile /g;" $@

-include $(SRC:.c=.d)
