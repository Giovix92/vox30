
SERVICES =    \"/tmp/services\"
INETD_CONF =  \"/tmp/inetd.conf\"

EXEC = inetd
OBJS = inetd.o config.o

CFLAGS += -DSERVICES=$(SERVICES) -DINETD_CONF=$(INETD_CONF)
CFLAGS += -Wall

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
	$(STRIP) inetd

clean:
	rm -f $(EXEC) $(OBJS)

install:
	cp $(EXEC) $(INSTALLDIR)/usr/sbin/

