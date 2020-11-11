AXIS_USABLE_LIBS = GLIBC UCLIBC
#include $(AXIS_TOP_DIR)/tools/build/Rules.axis

PROG = start-stop-daemon

all: $(PROG)

install: all
	install -m 0755 $(PROG) $(BUILDPATH)/PACKAGE/sbin/
	install -m 0755 $(PROG) $(INSTALLDIR)/sbin/

clean:
	rm -f $(PROG)
