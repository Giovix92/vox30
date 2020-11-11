MODULE:=ubus

SOURCE:=$(wildcard *.c) $(wildcard builtin/*.c)
DYNCFLAGS:=-I..

all: shared
shared: $(MODULE).so
static: $(SOURCE:.c=.o)

$(MODULE).so: $(SOURCE)
	$(CC) $(CFLAGS) $(SFLAGS) $(WFLAGS) $(DYNCFLAGS) $(LDFLAGS) $(DYNLDFLAGS) -fpic -fvisibility=hidden -shared -o $@ $+

%.o: %.c
	$(CC) $(CFLAGS) $(SFLAGS) $(WFLAGS) $(DYNCFLAGS) -c -o $(notdir $@) $+

clean:
	rm -f *.so *.o

