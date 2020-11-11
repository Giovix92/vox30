ifeq ($(CONFIG_SUPPORT_CORE_DUMP), 1)
CFLAGS  += -O -g -Wall -I$(KERNEL_DIR)/include
else
CFLAGS  += -Os -Wall  -I$(KERNEL_DIR)/include
endif
ifeq ($(CONFIG_SUPPORT_BUILD_DEBUG), 1)
CFLAGS  += -D__DEBUG__
endif
CFLAGS  += -I$(PRIVATE_APPSPATH)/include
CFLAGS  += -I$(PRIVATE_APPSPATH)/apps/cwmp_patch
CFLAGS  += -I$(PUBLIC_APPSPATH)/include
CFLAGS  += -L$(TARGETDIR)/lib -lslog 
ifeq ($(CONFIG_SUPPORT_UBUS), 1)
CFLAGS += -I$(PUBLIC_APPSPATH)/libs/ubus/ubus/ubus/
CFLAGS += -I$(PUBLIC_APPSPATH)/libs/ubus/libubox/libubox/include
CFLAGS += -I$(PUBLIC_APPSPATH)/libs/ubus/json-c/json-c-0.12/.libs/
CFLAGS += -lubus -lblobmsg_json -ljson-c -lubox -lnv
endif

CFLAGS	+=  -DENABLE_PIDFILE -DEMBED -DSCM_BINDING -D__SC_BUILD__ -DFAILOVER_ALG1 -DENABLE_TCP

all:	dnrd

clean:
	rm -f dnrd core tags ID *.o *.d *~ *.bak *.gdb

install: dnrd
	@echo "dnrd copyed"
	cp -v dnrd $(INSTALL_ROOT)/usr/sbin
	$(STRIP) $(INSTALL_ROOT)/usr/sbin/dnrd

sources = args.c check.c dns.c lib.c master.c query.c relay.c srvnode.c udp.c \
	  cache.c common.c domnode.c main.c qid.c rand.c sig.c tcp.c clsock.c 

	  

#include $(sources:.c=.d)

dnrd:	$(sources:.c=.o)
	$(CC) $(CFLAGS)  $^ -o $@
	
%.o: %.c
	$(CC) $(CFLAGS)  -o $@ -c $^ 

%.d: %.c
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $< \
		      | sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		      [ -s $@ ] || rm -f $@'
