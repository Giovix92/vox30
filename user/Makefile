
include $(APPSPATH)/libs.mk

SUBDIRS =
SUBDIRS += debug
SUBDIRS += public/libs
ifeq ($(SC_MULTIWAN),1)
SUBDIRS += public/apps/iproute
endif
ifneq ($(GPL), 1)
SUBDIRS += private
else
SUBDIRS += private/libs/libcurl
endif
SUBDIRS += public/apps
ifeq ($(CONFIG_SUPPORT_PLUME),1)
SUBDIRS += plumewifi
endif

all:
	echo $(X_PRIVATE_LIBS)
	make applications
	
build_gpl:
	make -C private build_gpl || exit 1 ;

applications::

	@for i in ${SUBDIRS} ;  do                                                    \
            echo "-----------------------------------------------------------------"; \
            echo -e "\033[;34m                         $$i                 \033[;0m"; \
            echo "-----------------------------------------------------------------"; \
            make -C $$i || exit 1 ;                     \
            make -C $$i install || exit 1 ;                                      \
	done

install:

clean:
	@for i in $(SUBDIRS) ; do $(MAKE) -C $$i clean || exit 1; done

