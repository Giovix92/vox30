CUR_PATH = $(shell pwd)
ifeq ($(CONFIG_SUPPORT_BUILD_DEBUG), 1)
CFLAGS += -g
else
CFLAGS += -s
endif
CFLAGS += -Os -std=gnu99 -D_GNU_SOURCE -Wall  -fPIC -D__SC_BUILD__
CFLAGS += -I$(CUR_PATH)/../rpc

LIBSAL_TARGET = libfonrpc.so

LIBSAL_OBJS += fonrpc.o fonapi.o 

all: $(LIBSAL_TARGET)

$(LIBSAL_TARGET) : $(LIBSAL_OBJS)
	$(CC) -Os $(CFLAGS) -shared -fPIC $(LDFLAGS) -o $@ -Wl,-soname,$@ $(LIBSAL_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
		
install :
	cp -f $(LIBSAL_TARGET) $(INSTALLDIR)/lib
	
clean:
	rm -rf $(LIBSAL_TARGET) $(LIBSAL_OBJS)
