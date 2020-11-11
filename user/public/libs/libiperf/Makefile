CFLAGS += -Os -Wall -s -fPIC 
CFLAGS += -I$(PUBLIC_APPSPATH)/include
OBJS = iperf_drv.o
BIN = libiperf_drv.so

all:$(BIN)

$(BIN):$(OBJS)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@
install:
	cp -f $(BIN) $(INSTALLDIR)/lib
build_gpl:
	cp -f $(BIN) $(GPL_INSTALLDIR)/lib

clean:
	-$(RM) $(BIN) $(OBJS)
