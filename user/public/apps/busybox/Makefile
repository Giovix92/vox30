

all:
ifeq ($(CONFIG_SUPPORT_5G_QD), 1)
	cd ./busybox-1.15.3 && cp .config.qd .config
endif
	cd ./busybox-1.15.3 && touch .config 
	cd ./busybox-1.15.3 && touch .config.old 
	cd ./busybox-1.15.3 && make 

install:
	cd ./busybox-1.15.3 && make install


clean:
	cd ./busybox-1.15.3 && make clean



