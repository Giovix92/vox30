#include ../../../Rules.mak
include ../prerules.mk
TARGET = libscdlna.so
OBJS = md5.o sample_util.o uuid.o sysdep.o
CFLAGS = -fPIC -I. -I../library/include \
		-I../mediaserver_dlna \
		-I../../libiconv.1.8/include

.PHONY:all

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) -shared -Wl,-soname,$@ $(LDFLAGS) $^ -o $@

install:
	cp -a ./lib*.so* $(T_LIB)
clean:
	$(RM) $(TARGET) $(OBJS)
