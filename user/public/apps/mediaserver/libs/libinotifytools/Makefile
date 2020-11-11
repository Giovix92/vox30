#Start of Makefile
#-----------------------------------------------------
#TOPDIR  :=  $(shell /bin/pwd)
#TOPDIR  :=  $(TOPDIR)/../..

#include $(TOPDIR)/prerules.mk
#-----------------------------------------------------
OBJS=inotifytools.o redblack.o
TARGET=inotify-objs

#LIBS = -lflash
all: $(TARGET)
%.o:%.c
	$(CC) $(CFLAGS) -I$(COMM_INC_PATH) -o $@ -c $^

$(TARGET):$(OBJS)

.PHONY:clean
clean:
	-rm -f *.[do] *.bak $(TARGET)
	-chmod 644 *.c Makefile

install:
	cp download $(T_USBIN)

