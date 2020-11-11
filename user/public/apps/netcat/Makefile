
CFLAGS = -D__SC_BUILD__ -I$(PRIVATE_APPSPATH)/include  
CFLAGS += -L$(TARGETDIR)/lib $(X_PRIVATE_LIBS) $(X_PUBLIC_LIBS)
all:
	cd ./netcat-0.7.1 &&  test -f Makefile || (find ./ |xargs touch && ./configure prefix=${INSTALLDIR} CC=${CC}  --host="mips-linux")
	cd ./netcat-0.7.1 &&  test -f Makefile && make 
	${STRIP} ./netcat-0.7.1/src/netcat
	install -D ./netcat-0.7.1/src/netcat ${INSTALLDIR}/usr/sbin/

 

install:
#	$(STRIP) ./netcat-0.7.1/src/netcat
#	install -m 755 ./netcat-0.7.1/src/netcat $(INSTALLDIR)/usr/sbin/   	
#	cp ./netcat-0.7.1/src/netcat $(INSTALLDIR)/usr/sbin/

clean:
	cd ./netcat-0.7.1 && test -f Makefile && make distclean
