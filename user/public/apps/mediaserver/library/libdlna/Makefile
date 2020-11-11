#-----------------------------------------------------
TOPDIR  :=  $(shell /bin/pwd)
TOPDIR  :=  $(TOPDIR)/../../

include $(TOPDIR)/prerules.mk
#-----------------------------------------------------

CFLAGS+=-Wall -s -Os -D_FILE_OFFSET_BITS=64 -I../ffmpeg-export-2008-01-17/include -I../jpeg-6b/
BUILD_RULES   += lib_static
INSTALL_RULES += install_static

LIBNAME = libdlna
LIBNAME_SHARED = ${LIBNAME}.so
LIBNAME_STATIC = ${LIBNAME}.a
LIBNAME_VERSION = $(LIBNAME_SHARED).$(VERSION)
LIBNAME_MAJOR = $(LIBNAME_SHARED).$(shell echo $(VERSION) | cut -f1 -d.)

SRCS =  \
	profiles.c \
	containers.c \
	image_jpeg.c \
	image_png.c \
	audio_aac.c \
	audio_ac3.c \
	audio_amr.c \
	audio_atrac3.c \
	audio_g726.c \
	audio_lpcm.c \
	audio_mp2.c \
	audio_mp3.c \
	audio_wma.c \
	av_mpeg1.c \
	av_mpeg2.c \
	av_mpeg4_part2.c \
	av_mpeg4_part10.c \
	av_wmv9.c \
	dlna.c \

EXTRADIST = \
	dlna.h \
	dlna_internals.h \
	upnp_internals.h \
	containers.h \
	profiles.h \

CFLAGS += -I../include


OBJS = $(SRCS:.c=.o)
LOBJS = $(SRCS:.c=.lo)

.SUFFIXES: .c .o .lo

all: $(BUILD_RULES)# TAGS tags

.c.o:
	$(CC) -c $(CFLAGS) $(OPTFLAGS) -o $@ $<

.c.lo:
	$(CC) -c -fPIC $(CFLAGS) $(OPTFLAGS) -o $@ $<

lib_static_info_pre:
	@echo 
	@echo "#############################################"
	@echo "#         Building static objects           #"
	@echo "#############################################"

lib_static_info_post:
	@echo 
	@echo "#############################################"
	@echo "#         Linking static objects            #"
	@echo "#############################################"

lib_static: lib_static_info_pre $(OBJS) lib_static_info_post
	$(AR) r $(LIBNAME_STATIC) $(OBJS)
	$(RANLIB) $(LIBNAME_STATIC)

lib_shared_info_pre:
	@echo 
	@echo "#############################################"
	@echo "#         Building shared objects           #"
	@echo "#############################################"

lib_shared_info_post:
	@echo 
	@echo "#############################################"
	@echo "#         Linking shared objects            #"
	@echo "#############################################"

lib_shared: lib_shared_info_pre $(LOBJS)lib_shared_info_post
	$(CC) -shared -Wl,-soname,$(LIBNAME_MAJOR) $(LOBJS) \
	  $(LDFLAGS) $(EXTRALIBS) -o $(LIBNAME_VERSION)
	$(LN) -sf $(LIBNAME_VERSION) $(LIBNAME_MAJOR)
	$(LN) -sf $(LIBNAME_MAJOR) $(LIBNAME_SHARED)

dlna-profiler: dlna-profiler.o
	$(CC) -o dlna-profiler dlna-profiler.o libdlna.a  -L$(COMM_LIB_PATH) -L../libavdevice -L../ffmpeg-export-2008-01-17/libavformat -L../ffmpeg-export-2008-01-17/libavcodec -L../ffmpeg-export-2008-01-17/libavutil   -lavformat -lavcodec -lavutil  -lm -ljpeg
	$(STRIP) $@

clean:
	-$(RM) -f *.o *.d *.lo *.a *.so*
	-$(RM) -f .depend


install:


.PHONY: clean depend dlna-profiler

