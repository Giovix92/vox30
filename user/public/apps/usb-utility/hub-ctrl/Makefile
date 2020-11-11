
TARGET = hub_ctrl
CFLAGS = -Wall -Werror -I$(PUBLIC_APPSPATH)/libs/libusb/libusb-0.1.12 -lusb
LDFLAGS = -L$(PUBLIC_APPSPATH)/libs/libusb/libusb-0.1.12/.libs/
TARGET_DIR = $(INSTALLDIR)/usr/sbin

OBJS = hub-ctrl.o

all: $(TARGET)


hub_ctrl: $(OBJS)
	$(CC) $(CFLAGS)  -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -c $< 

install:
	$(STRIP) $(TARGET)
	install -D  $(TARGET) $(TARGET_DIR)

clean:
	rm -f $(OBJS) $(TARGET)
