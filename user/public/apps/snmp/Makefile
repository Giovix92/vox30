
CFLAGS += -D__SC_BUILD__


CFLAGS += -L$(INSTALLDIR)/lib $(X_PRIVATE_HCAL_LIBS) $(X_PRIVATE_SAL_LIBS) $(X_PRIVATE_DLS_LIBS) -lslog
all:
	cd ./net-snmp-5.0.8 && make

install:
	cd ./net-snmp-5.0.8 && make install

clean:
	cd ./net-snmp-5.0.8 && make clean

