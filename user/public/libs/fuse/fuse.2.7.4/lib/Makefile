#include ../../../../Rules.mak
#CROSS_PREFIX = $(CROSS)

#CC = $(CROSS_PREFIX)gcc
#LD = $(CROSS_PREFIX)ld
#STRIP = $(CROSS_PREFIX)strip

CFLAGS := -DHAVE_CONFIG_H -I. -I../include -I../include \
	-DFUSERMOUNT_DIR=\"/usr/sbin\" -D_FILE_OFFSET_BITS=64 \
	-D_REENTRANT -DFUSE_USE_VERSION=26 -Wall -Werror \
	-W -Wno-sign-compare -Wstrict-prototypes -Wmissing-declarations  \
	-Wwrite-strings -O2 -fno-strict-aliasing \
	-fPIC -DPIC
ifeq ($(CONFIG_SUPPORT_BUILD_DEBUG), 1)
CFLAGS += -g 
endif
LDFLAGS  := -shared
LDFLAGS  += -lrt -ldl  -pthread -Wl,--version-script \
	-Wl,./fuse_versionscript

#iconv_source = modules/iconv.c
#subdir_source = modules/subdir.c
mount_source = mount.c mount_util.c

FUSESRC  := fuse.c		\
	fuse_kern_chan.c	\
	fuse_loop.c		\
	fuse_loop_mt.c		\
	fuse_lowlevel.c		\
	fuse_mt.c		\
	fuse_opt.c		\
	fuse_session.c		\
	fuse_signals.c		\
	helper.c		\
	$(subdir_source)	\
	$(iconv_source)		\
	$(mount_source)

FUSEOBJS := $(patsubst %.c,%.o,$(FUSESRC))
LIBFUSE  := libfuse.so.2.7.4 libfuse.so.2 libfuse.so

SRC := $(FUSESRC)

LIBS := $(LIBFUSE)

all: $(LIBS)

libfuse.so.2.7.4: $(FUSEOBJS)
	$(CC) $(LDFLAGS) -Wl,-soname -Wl,libfuse.so.2 $(FUSEOBJS) -o $@
	$(STRIP) $@

libfuse.so.2 libfuse.so: libfuse.so.2.7.4
	ln -sf $< $@

install:
	-cp -a $(LIBS) $(INSTALLDIR)/lib/

clean:
	rm -f *.o *.d  $(LIBS)

%.d: %.c
	@$(CC) $(CFLAGS) -MM $< > $@
	@sed -i "1s/^$*.o:/$*.o $*.d: Makefile /g;" $@

-include $(SRC:.c=.d)
