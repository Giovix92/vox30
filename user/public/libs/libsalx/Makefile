
CUR_PATH = $(shell pwd)

ifeq ($(CONFIG_SUPPORT_BUILD_DEBUG), 1)
CFLAGS += -g
else
CFLAGS += -s
endif
CFLAGS += -Os -Wall  -fPIC
CFLAGS += -I$(PRIVATE_APPSPATH)/include -I$(PRIVATE_APPSPATH)/include/sal 
CFLAGS += -I$(KERNEL_DIR)/include

ifeq ($(CONFIG_SUPPORT_DMALLOC),1)
#DMALLOC=1
ifeq ($(DMALLOC),1)
CFLAGS += -include $(DMALLOC_DIR)/dmalloc_inc_file.h
CFLAGS += -DDMALLOC -DDMALLOC_FUNC_CHECK
DMALLOC_LIB=$(DMALLOC_DIR)/libdmalloc.a
endif
endif
LIBSAL_TARGET = libsalx.so

LIBSAL_OBJS += wan.o ntp.o misc.o 
LIBSAL_OBJS += traceroute.o
LIBSAL_OBJS += nslookup.o
ifeq ($(CONFIG_SUPPORT_IPERF), 1)
LIBSAL_OBJS += iperf_diagnostics.o
endif
ifeq ($(CONFIG_SUPPORT_IPV6), 1)
IPROUTEPATH = $(PUBLIC_APPSPATH)/libs/libnetlink
CFLAGS += -I$(IPROUTEPATH)/include -I$(IPROUTEPATH)/lib
LIBNETLINK=$(IPROUTEPATH)/lib/libnetlink.so $(IPROUTEPATH)/lib/libutils.so
#LIBSAL_OBJS += route.o
endif
ifeq ($(CONFIG_SUPPORT_TR111), 1)
LIBSAL_OBJS +=tr111.o
endif
all: $(LIBSAL_TARGET)

$(LIBSAL_TARGET) : $(LIBSAL_OBJS)
	$(CC) -Os  -shared -fPIC $(LDFLAGS) -o $@ -Wl,-soname,$@ $(LIBSAL_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
		
install :
	cp -f $(LIBSAL_TARGET) $(INSTALLDIR)/lib
	
clean:
	rm -rf $(LIBSAL_TARGET) $(LIBSAL_OBJS)

