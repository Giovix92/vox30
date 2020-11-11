MODULE:=fonrpc

SOURCE:=$(wildcard *.c)
DYNCFLAGS:=-I..
OBJECTS:=$(SOURCE:.c=.o)

all: shared
shared: $(MODULE).so
static: $(SOURCE:.c=.o)

$(MODULE).so: $(SOURCE)
	$(CC) $(CFLAGS) $(SFLAGS) $(WFLAGS) $(DYNCFLAGS) $(LDFLAGS) $(DYNLDFLAGS) -fpic -fvisibility=hidden -shared -o $@ $+

%.o: %.c
	$(CC) $(CFLAGS) $(SFLAGS) $(WFLAGS) $(DYNCFLAGS) -c -o $(notdir $@) $+

clean:
	rm -f *.so *.o
