ALLJOYN_PATH   =$(PUBLIC_APPSPATH)/apps/alljoyn/alljoyn-14.12.00b
ALLJOYN_BUILD_PATH =build/linux/mips/debug
ALLJOYN_CXX_PATH  =$(ALLJOYN_PATH)/$(ALLJOYN_BUILD_PATH)/dist


all:
	scons WS=off CPU=mips OS=linux BINDINGS=cpp CROSS_COMPILE=$(TOOLPATH)/$(TOOLPREFIX) ALLJOYN_DISTDIR=$(ALLJOYN_CXX_PATH) BUILD_SERVICES_SAMPLES=off

install:

clean:
	scons -c BINDINGS=cpp CPU=mips && \
	rm -rf build

