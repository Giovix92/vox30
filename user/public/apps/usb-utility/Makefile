
all:
ifeq ($(CONFIG_SUPPORT_3G),1)
	cd ./usb-modeswitch-1.2.5 && test -f Makefile && make 
endif
	cd ./hub-ctrl && test -f Makefile && make && make install
install:
ifeq ($(CONFIG_SUPPORT_3G),1)
	${STRIP} ./usb-modeswitch-1.2.5/usb_modeswitch
	install -D ./usb-modeswitch-1.2.5/usb_modeswitch ${INSTALLDIR}/usr/sbin
endif
clean:
ifeq ($(CONFIG_SUPPORT_3G),1)
	cd ./usb-modeswitch-1.2.5 && make clean
endif
	cd ./hub-ctrl && make clean
