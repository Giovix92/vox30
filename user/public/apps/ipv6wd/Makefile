CFLAGS += -Os  -Wall 

ifeq ($(CONFIG_SUPPORT_BUILD_DEBUG), 1)
CFLAGS += -g
else
CFLAGS += -s
endif

IPROUTEPATH = $(PUBLIC_APPSPATH)/libs/libnetlink
CFLAGS += -I$(PUBLIC_APPSPATH)/include
CFLAGS += -I$(PRIVATE_APPSPATH)/include
LDFLAGS = -L$(TARGETDIR)/lib -lnv -lslog -lsalx -L$(IPROUTEPATH)/lib -lnetlink -lutils
DEFINES= -DRESOLVE_HOSTNAMES
CFLAGS += -I$(IPROUTEPATH)/include $(DEFINES) -I$(IPROUTEPATH)/lib 
ifneq ($(BRCM_502), 1)
CFLAGS += -I$(KERNELDIR)/include
else
CFLAGS += -idirafter$(KERNELDIR)/include
endif

BIN = ipv6wd
OBJS += ipaddress.o ipmonitor.o iproute.o 

all : $(BIN)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(LDFLAGS)
main.o: main.c
	$(CC) -c $(CFLAGS) $^ -o $@ $(LDFLAGS)
$(BIN): main.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

install:
	install $(BIN) $(INSTALLDIR)/usr/sbin/$(BIN)
	$(STRIP) $(INSTALLDIR)/usr/sbin/$(BIN)

clean:
	rm -f *.o $(BIN)
	
