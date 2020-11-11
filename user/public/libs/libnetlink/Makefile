#CCOPTS = -D_GNU_SOURCE -O2 -Wstrict-prototypes -Wall
#CFLAGS = $(CCOPTS) -I../include $(DEFINES)
ifeq ($(BRCM_502), 1)
CFLAGS += $(CCOPTS) -I../include $(DEFINES) 
else
CFLAGS += $(CCOPTS) -I../include $(DEFINES) -I$(KERNELDIR)/include
endif
YACCFLAGS = -d -t -v

SUBDIRS=lib 

LIBNETLINK=../lib/libnetlink.so ../lib/libutils.so

all:
	@set -e; \
	for i in $(SUBDIRS); \
	do $(MAKE) -C $$i; done

install:
	@set -e; \
	for i in $(SUBDIRS); \
	do $(MAKE) -C $$i install; done

clean:
	@for i in $(SUBDIRS); \
	do $(MAKE) $(MFLAGS) -C $$i clean; done


