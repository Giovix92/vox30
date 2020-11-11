#***********************************************************************
#
#  Copyright (c) 2012  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/

all dynamic install: conditional_build

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#

APP = strace-4.8

clean:
	@if [ -e $(APP)/Makefile ]; then \
	cd $(APP); $(MAKE) clean ; \
	fi
	-rm -f $(APP)/Makefile 

# The next line is a hint to our release scripts
# GLOBAL_RELEASE_SCRIPT_CALL_DISTCLEAN
distclean: clean
	rm -rf $(APP)

#.PHONY: check_untar_patch_configure conditional_build
check_untar_patch_configure:
	cd $(APP) && test -f Makefile || ./configure --host=$(TOOLCHAIN_PREFIX) \
        --target=$(TOOLCHAIN_PREFIX) \
        --disable-developer \

conditional_build: check_untar_patch_configure
	cd $(APP); $(MAKE) 
	install -m 755 $(APP)/strace  $(INSTALLDIR)/bin/strace
