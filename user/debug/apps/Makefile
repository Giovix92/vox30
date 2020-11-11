
SUBDIRS =

ifeq ($(SCM_GPON_US_QOS), 1)
SUBDIRS += qos
endif
ifeq ($(CONFIG_SUPPORT_DEBUG), 1)
SUBDIRS += strace
endif
#SUBDIRS += tcpdump
SUBDIRS += sysstat
#SUBDIRS += sc_mark
SUBDIRS += gzip
#SUBDIRS += capture
ifeq ($(CONFIG_SUPPORT_DEBUG), 1)
#SUBDIRS += valgrind
endif

# $(CONFIG_SUPPORT_DMALLOC)=1 or $(CONFIG_SUPPORT_VOIP_DMALLOC)=1
ifneq ($(CONFIG_SUPPORT_DMALLOC)_$(CONFIG_SUPPORT_VOIP_DMALLOC), 0_0)
SUBDIRS += dmalloc
endif

all:
	make applications

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

