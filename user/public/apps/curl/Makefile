ifeq ("x$(X_PRIVATE_LIBS)", "x")
include $(APPSPATH)/libs.mk
endif
CFLAGS += -O0 -Wall -g 
CFLAGS += -I$(PUBLIC_APPSPATH)/include
CFLAGS += -I$(PRIVATE_APPSPATH)/libs/libcal/include/xml
CFLAGS += -I$(PRIVATE_APPSPATH)/include
CFLAGS += -I$(PRIVATE_APPSPATH)/libs/libcurl/curl-7.47.0/include/
LDFLAGS += -L$(PRIVATE_APPSPATH)/libs/libcurl/curl-7.47.0/lib/.libs -L$(INSTALLDIR)/lib  -lcurl -lssl -lcrypto
BIN = curl_s
OBJS  = curl.o 
all: $(BIN)
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 


%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

install:
	install -D $(BIN) $(INSTALLDIR)/bin/

build_gpl:
	install $(BIN) $(GPL_INSTALLDIR)/usr/sbin/$(BIN)

clean:
	rm -f *.o $(BIN)

