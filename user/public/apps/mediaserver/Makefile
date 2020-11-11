#include ../../Rules.mak
#export _CROSS
#export CROSS_COMPILE=$(CROSS)
CFLAGS = -D__SC_BUILD__
#export FF_PLATFORM = x86
#export FF_PLATFORM = mips
#export FF_PLATFORM = arm
ifeq ($(ARCH), arm)
export FF_PLATFORM = arm
else
export FF_PLATFORM = mips
endif
USE_MINIDLNA=1
ifeq ($(USE_MINIDLNA),1)
	SUBLIBS = library/ffmpeg-3.1.1
#	SUBLIBS += library/zlib-1.2.3
	SUBLIBS += library/sqlite-3.5.9
	SUBLIBS += library/jpeg-6b
	SUBLIBS += library/libogg-1.3.0/src
	SUBLIBS += library/libexif-0.6.20/libexif
	SUBLIBS += library/flac-1.2.1/src/libFLAC
	SUBLIBS += library/libid3tag-0.15.1b
	SUBLIBS += library/libvorbis-1.3.2/lib
#SUBLIBS += library/libpng-1.2.18
	SUBLIBS += library/libpng-1.6.25
	SUBLIBS += library/tiff-4.0.3
	SUBDIRS += minidlna-1.0.24
else

SUBLIBS = library/ffmpeg-export-2008-01-17
SUBLIBS += library/zlib-1.2.3
SUBLIBS += library/sqlite-3.5.9
SUBLIBS += library/jpeg-6b
SUBLIBS	+= library/libdlna
SUBLIBS += library/giflib-4.1.4
SUBLIBS += library/libpng-1.2.18
ifneq ($(GPL),1)
SUBLIBS += library/libupnp-1.6.0_dlna
SUBLIBS += libs
SUBDIRS	= mediaserver_dlna
endif

endif
.PHONY:libs

all: libs dirs

libs:${SUBLIBS}
	touch "library/libogg-1.3.0/include/ogg/Makefile"
	touch "library/libogg-1.3.0/Makefile"
	touch "library/libogg-1.3.0/doc/Makefile"
	touch "library/libogg-1.3.0/doc/libogg/Makefile"
	touch "library/libogg-1.3.0//config.status"
	touch "library/libogg-1.3.0/src/Makefile"
	touch "library/jpeg-6b/jconfig.h"
	touch "library/libvorbis-1.3.2/lib/Makefile"
	touch "library/libexif-0.6.20/libexif/Makefile"
	@for i in ${SUBLIBS} ;  do                                                    \
        echo "-----------------------------------------------------------------"; \
	echo -e "\033[;34m                         $$i                 \033[;0m"; \
	echo "-----------------------------------------------------------------"; \
	cd $$i && ./build.sh || exit 1;                                                     \
    cd - ;                                                                    \
	done

dirs : ${SUBDIRS}
	@for i in ${SUBDIRS} ;  do                                                    \
	echo "-----------------------------------------------------------------"; \
	echo -e "\033[;34m                         $$i                 \033[;0m"; \
	echo "-----------------------------------------------------------------"; \
	make -C $$i || exit 1 ;                                                   \
	done

install:
	for i in ${SUBLIBS} ; do make -C $$i install || exit 1 ; done
	for i in ${SUBDIRS} ; do make -C $$i install || exit 1 ; done
		
clean:
	for i in ${SUBLIBS} ; do make -C $$i clean ; done
	for i in ${SUBDIRS} ; do make -C $$i clean ; done
	rm -f library/lib/*
	


#=====================================================================================
