CC ?= gcc
CFLAGS += -g -std=c99 -Wall
CFLAGS += -I$(PUBLIC_APPSPATH)/include/ 
LDFLAGS =
LDFLAGS += -L$(TARGETDIR)/lib $(X_PUBLIC_LIBS) -lnv
LIBS = -lm
ifeq ($(CONFIG_SUPPORT_WEBAPI), 1)
LIBS += -lsalx
endif
STRIP ?= strip
INSTALL = install
OBJS = src/Speedtest.c \
	src/SpeedtestConfig.c \
	src/SpeedtestServers.c \
	src/url.c \
	src/http.c

all: speedtest

speedtest: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(LDFLAGS)

install:
	$(STRIP) speedtest
	$(INSTALL) speedtest $(INSTALLDIR)/usr/sbin

#test:	speedsest
#	valgrind --leak-check=full --show-leak-kinds=all ./speedsest --server http://speedtest.skynet.net.pl/speedtest/upload.php
#	valgrind --leak-check=full --show-leak-kinds=all ./speedsest

clean:
	rm speedtest

.PHONY: all
