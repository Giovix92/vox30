#-----------------------------------------------------
TOPDIR  :=  $(shell /bin/pwd)
TOPDIR  :=  $(TOPDIR)/..

#include ../../../Rules.mak
include $(TOPDIR)/prerules.mk
#-----------------------------------------------------

#RAMDISK_DIR =../../../image/target/usr
TARGET_PREFIX = $(TOOLPREFIX)

INCLUDES= -I../libs -I$(COMM_INC_PATH) -I$(COMM_INC_PATH)/upnp \
		-I../../libiconv.1.8/include \
		-I../library/ffmpeg-export-2008-01-17/include \
		-I../library/libdlna/ \
		-I../library/zlib-1.2.3  \
		-I../library/libpng-1.2.18

LIBS+=-L../libs -lscdlna \
	  -L../../libiconv.1.8/lib \
	  -L../library/lib \
	  -L../library/libdlna \
	  -L../library/ffmpeg-export-2008-01-17/libavdevice \
	  -L../library/ffmpeg-export-2008-01-17/libavformat \
	  -L../library/ffmpeg-export-2008-01-17/libavcodec \
	  -L../library/ffmpeg-export-2008-01-17/libavutil \
	  -ldlna -lavformat -lavcodec -lavutil -lm -lpthread -ljpeg -lz -lpng12 -lsqlite3 -lupnp -lthreadutil -lixml -liconv

OBJS = upnpd.o tool.o mediaserver.o if_info.o make_upnp_xml.o http.o  pic_scale.o
OBJS2 = mmsio.o sample_util.o  tool.o mediaserver.o md5.o uuid.o sysdep.o if_info.o make_upnp_xml.o http.o libinotifytools/inotifytools.o libinotifytools/redblack.o pic_scale.o
OBJS3 = mmsio.o ../libs/libinotifytools/inotifytools.o ../libs/libinotifytools/redblack.o

ifeq ($(DEBUG),1)
OPT = -g -O2
else
OPT = -O2 
endif

CFLAGS += -Wall -g $(OPT)
CFLAGS += -s -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D__ICONV_SUPPORT__
LDFLAGS += -Wall -g
#CFLAGS += -D__PNG_DLNA_SUPPORT__
#CFLAGS += -D__DEBUG__
CFLAGS += -D_SRT_SUPPORT_
APPS = mediaupnp

EARLY_ADD=1
ifeq ($(EARLY_ADD),1)
CFLAGS += -D_EARLY_ADD_
endif

all: $(APPS) 

mediaupnp:  $(OBJS) $(OBJS3)
	make -C ../libs/libinotifytools
	$(CC) -print-file-name=libc.so 
	$(CC)  $(LDFLAGS) $(OBJS) $(OBJS3) $(LIBS) -o  $@  

mmsio: $(OBJS2)
	make -C ../libs/libinotifytools
	$(CC) $(LDFLAGS) $(OBJS2) $(LIBS) -o $@ 

%.o:	%.c
	$(warning $(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	make -C ../libs/libinotifytools clean
	rm -f *.o *.d  $(APPS) core *~

install:
	$(STRIP) $(APPS)
	[ -d $(RAMDISK_DIR)/usr/etc/resource/ ] || mkdir -p $(RAMDISK_DIR)/usr/etc/resource
	mkdir -p $(RAMDISK_DIR)/usr/etc/resource
	cp -fa ../upnp_xml/*  $(RAMDISK_DIR)/usr/etc/resource/
	rm -rf $(RAMDISK_DIR)/usr/etc/resource/CVS
	rm -rf $(RAMDISK_DIR)/usr/etc/resource/ContentDirectory/CVS
	rm -rf $(RAMDISK_DIR)/usr/etc/resource/ConnectionManager/CVS
	rm -rf $(RAMDISK_DIR)/usr/etc/resource/images/CVS
	#cp -af $(APPS) $(T_USBIN)
	cp -f  $(APPS) ${RAMDISK_DIR}/usr/sbin
	ln -sf /usr/sbin/$(APPS) $(RAMDISK_DIR)/usr/etc/$(APPS)
