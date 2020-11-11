CFLAGS += -Os -Werror -Wall -s -I$(PUBLIC_APPSPATH)/include -fPIC

OBJS = slog.o
BIN = libslog.so

all:$(BIN)

$(BIN):$(OBJS)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@
install:
	cp -f $(BIN) $(INSTALLDIR)/lib

clean:
	-$(RM) $(BIN) $(OBJS)
