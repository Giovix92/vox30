MODULE:=fonctl

SOURCE:=$(wildcard *.c)
DYNCFLAGS:=-I..
OBJECTS:=$(SOURCE:.c=.o)

all: shared
shared: $(MODULE).so
static: $(SOURCE:.c=.o)
	ln -sf hotspotd ../fonctl

$(MODULE).so: $(SOURCE)
	@echo This module cannot be build as a shared object.
	@false

%.o: %.c
	$(CC) $(CFLAGS) $(SFLAGS) $(WFLAGS) $(DYNCFLAGS) -c -o $(notdir $@) $+

clean:
	rm -f *.so *.o ../fonctl
