MODULE:=tunnel
TUNNEL_FEATURES:=-DWITH_TUNNEL_L2TP

SOURCE:= tunnel.c l2tp.c
DYNCFLAGS:=-I.. $(TUNNEL_FEATURES)
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
