top_srcdir	= ..
srcdir		= .

CFLAGS += -Os -Wall -s -O2 -Dlinux -D__SC_BUILD__
CFLAGS += -I$(top_srcdir)/include -I../include \
		  -I.. -I$(srcdir)/.. -I$(srcdir)/../snmplib \
		  -I$(srcdir)/../agent -I../agent/helpers \
		  -I$(srcdir)/../agent/mibgroup \
		  -I$(top_srcdir)/../include \

ifeq ($(CONFIG_SUPPORT_BUILD_DEBUG), 1)
CFLAGS += -g 
endif
#LDFLAGS	+= -L$(INSTALLDIR)/lib -lnv -lz -lcal -lcrypt -lcml_api -lsecxal
#LDFLAGS	+= -L$(INSTALLDIR)/lib -lapcfg -lflashlog -ldummy_nvram -lscfg -lutility -lcalv2 -lcal -lmd5 -lcrypt -lcml_api -lservicectrl -ldns
#LDFLAGS += -lsal -lrcl -lm
#LDFLAGS += -lsal -lrcl -lm -lsyslog

BIN = #sc_snmptrap 
BIN2 = snmptrap

all: $(BIN2)

install:
	install -D $(BIN2) $(INSTALLDIR)/usr/sbin/$(BIN)
	$(STRIP) $(INSTALLDIR)/usr/sbin/$(BIN2)

#snmptrap modify by net-snmp open Src
snmptrap: snmptrap.o 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -L../snmplib/ -lnetsnmp 

#sc_snmptrap support an interface for user to command, It use snmptrap to come true
sc_snmptrap: sc_snmptrap.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ sc_snmptrap.o -lnvram -lsecxal
#$(CC) $(CFLAGS) $(LDFLAGS) -o $@ sc_snmptrap.o -lnvram -lapcfg -lutility -lscfg -I../../../../private/include/calv2

clean:
	rm -f $(BIN) $(BIN2) *.o

