ifeq ($(CONFIG_SUPPORT_SAMBA_VERSION_36), 1)
	SUBDIR = samba-3.6.22
else
	SUBDIR = samba-3.0.37
endif

all:
	cd $(SUBDIR) && make

install:
	cd $(SUBDIR) && make install

clean:
	cd $(SUBDIR) && make clean
