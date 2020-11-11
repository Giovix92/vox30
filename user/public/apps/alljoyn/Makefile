OPENSSL_CPPPATH        =$(OPENSSL_DIR)/include
OPENSSL_LIBPATH        =$(OPENSSL_DIR)
ALLJOYN_C_LIB_PATH     =$(PUBLIC_APPSPATH)/apps/alljoyn/alljoyn-14.12.00b/$(ALLJOYN_BUILD_PATH)
ALLJOYN_BUILD_PATH     =build/linux/mips/debug/dist/cpp/lib
ALLJOYN_C_APP_PATH     =$(ALLJOYN_C_LIB_PATH)/../bin
ALLJOYN_SERVICE_API_PATH =alljoyn-14.12.00b/alljoyn-services-API/services/base

ALLJOYN_C_APP = aclient
ALLJOYN_C_LIB = liballjoyn.so


all:
	cd alljoyn-14.12.00b && \
	scons WS=off CPU=mips OS=linux BINDINGS=c,cpp CROSS_COMPILE=$(TOOLPATH)/$(TOOLPREFIX) OPENSSL_CPPPATH=$(OPENSSL_CPPPATH)  OPENSSL_LIBPATH=$(OPENSSL_LIBPATH)
	cd $(ALLJOYN_SERVICE_API_PATH)/config && make
	cd $(ALLJOYN_SERVICE_API_PATH)/notification && make
	cd $(ALLJOYN_SERVICE_API_PATH)/services_common && make
	cd $(ALLJOYN_SERVICE_API_PATH)/controlpanel && make

install:
	cd alljoyn-14.12.00b && cp -f $(ALLJOYN_C_LIB_PATH)/$(ALLJOYN_C_LIB) $(INSTALLDIR)/lib \
 	&& cp -f $(ALLJOYN_C_APP_PATH)/$(ALLJOYN_C_APP) $(INSTALLDIR)/usr/sbin \
	&& cp -f $(CROSS_DIR)/usr/mips-linux-uclibc/lib/libstdc++.so.6.0.13 $(INSTALLDIR)/lib \
	&& cd $(INSTALLDIR)/lib && ln -s libstdc++.so.6.0.13 libstdc++.so.6

clean:
	cd alljoyn-14.12.00b &&  scons -c BINDINGS=c,cpp CPU=mips OPENSSL_CPPPATH=$(OPENSSL_CPPPATH) OPENSSL_LIBPATH=$(OPENSSL_LIBPATH) \
	&& rm -rf build
	cd $(ALLJOYN_SERVICE_API_PATH)/config && make clean
	cd $(ALLJOYN_SERVICE_API_PATH)/notification && make clean
	cd $(ALLJOYN_SERVICE_API_PATH)/services_common && make clean
	cd $(ALLJOYN_SERVICE_API_PATH)/controlpanel && make clean


