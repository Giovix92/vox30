SUBDIR = libpam-1.3.1
all:
	cd $(SUBDIR) && ./configure CC=$(CC) --host=arm-linux
	cd $(SUBDIR) && make

install:
	cd $(SUBDIR) && make install

clean:
	cd $(SUBDIR) && make clean



