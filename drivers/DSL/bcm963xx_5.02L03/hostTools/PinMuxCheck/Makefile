
vpath %.c ../../shared/opensource/boardparms/bcm963xx
vpath %.h ../../shared/opensource/boardparms/bcm963xx

CFLAGS +=  -I ../../shared/opensource/include/bcm963xx/ -DCONFIG_BCM9$(BRCM_CHIP) -DPINMUXCHECK -DCONFIG_BOARDPARMS_VOICE -D_CFE_ -DPRINT_ERRORS 

bptest: bptest.c boardparms.c boardparms_voice.c bp_funcs.c

clean:
	rm -f *.o bptest

