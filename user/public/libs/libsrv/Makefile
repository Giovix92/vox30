LIB  = libsrv.so
OBJS = libsrv.o patch.o
TOP_DIR = $(shell pwd)
TARGETDIR = $(INSTALLDIR)
BUILD_DIR=$(BSP_DIR)
CFLAGS += -I$(TOP_DIR)/include -I$(TOP_DIR)/include/arpa

all: install

clean: uninstall
	rm -f *.o *.so example/libsrv.so*
	#rm -f $(BUILD_DIR)/userspace/public/include/libsrv.h
	rm -f $(TARGETDIR)/lib/libsrv.so

%.o.%.c:
	$(CC) $(CFLAGS) -c $^

libsrv.so: $(OBJS)
	@echo "Melinda libsrv Makefile Copy libsrv.so to BUILD_DIR=$(BUILD_DIR)..."
	$(CC) -shared -Iinclude -Wl,--whole-archive,-soname,$@ -o $@ $(OBJS) -lc -Wl,--no-whole-archive
# Remove -lresolv	
# uclibc libresolv is stub, don't link it just put the functions we need in patch.c
#	$(CC) -shared -Iinclude -o libsrv.so libsrv.c -lc -lresolv


install: $(LIB)
	install libsrv.h $(TOP_DIR)/../../include
	install libsrv.h ./include
	@echo "Melinda libsrv Makefile Copy libsrv.so to $(TARGETDIR)..."
	install libsrv.so $(TARGETDIR)/lib
	@echo "Melinda libsrv Makefile Copy libsrv.so to $(TARGETDIR) done"


#	@echo "Melinda --- Copy libsrv.so to $(FS_DIR)/lib..."
#	cp -af $(BCM_DIR)/libsrv.so $(FS_DIR)/lib/
#	@echo "Melinda --- Create symbolic link to  $(FS_DIR)/lib/libsrv.so ..."
#	ln -sf libsrv.so   $(FS_DIR)/lib/libsrv.so.1
#	ln -sf libsrv.so.1 $(FS_DIR)/lib/libsrv.so.1.0
#	@echo "Melinda --- Create symbolic link to  $(FS_DIR)/lib/libsrv.so done"

#	install -d $(TARGETDIR)/lib
#	install libsrv.so $(TARGETDIR)/lib/
#	$(STRIP) $(TARGETDIR)/lib/libsrv.so
#	rm -f $(TARGETDIR)/lib/libsrv.so.1 $(TARGETDIR)/lib/libsrv.so.1.0
#	ln -s libsrv.so   $(TARGETDIR)/lib/libsrv.so.1
#	ln -s libsrv.so.1 $(TARGETDIR)/lib/libsrv.so.1.0


uninstall:
	#@echo 'libsrv uninstall BCM_DIR=$(BCM_DIR)'
	#@echo 'FS_DIR=$(FS_DIR)'
	#rm -f $(INSTALL_DIR)/lib/public/$(LIB)
	#rm -f $(BCM_DIR)/lib/libsrv.so*
	#rm -f $(FS_DIR)/lib/libsrv.so*
	sudo rm -f $(TARGETDIR)/lib/libsrv.so* 

include $(BUILD_DIR)/make.common
include $(BUILD_DIR)/sc.mak
