ifneq ($(wildcard Makefile.fullsrc),)

include Makefile.fullsrc

else

EXE = beepPkgBuilder

dynamic:$(EXE) 

clean:
	rm -f $(EXE)

$(EXE):
ifeq ($(shell uname -m),x86_64)
	cp $(EXE).x64.save $(EXE)
else
	cp $(EXE).x32.save $(EXE)
endif
	
#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /hostTools, /hostTools,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common
endif
