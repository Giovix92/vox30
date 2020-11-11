
APP = ethtool-4.19

all dynamic install: conditional_build

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /user, /user,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
$(info BUILD_DIR=$(BUILD_DIR))

# include $(BUILD_DIR)/make.common

UNTAR_COMPLETE_FILE = .untar_complete
CONFIG_COMPLETE_FILE = config_complete

MAKE_ARGS=CROSS_COMPILE= WITH_SRV=no WITH_UUID=no prefix=/

#export LDFLAGS := $(OPENSSL_LDFLAGS)

$(APP)/$(UNTAR_COMPLETE_FILE): $(APP).tar.gz Makefile
	rm -rf $(APP)
	tar xf $(APP).tar.gz
	(cd $(APP); for PATCH in ../*.patch; do if [ -e "$$PATCH" ]; then patch -p1 < $$PATCH; fi; done)
	@echo "$(APP) is untarred"
	env
	touch $(APP)/$(UNTAR_COMPLETE_FILE)

$(APP)/$(CONFIG_COMPLETE_FILE):# $(APP)/$(UNTAR_COMPLETE_FILE) 
	(cd $(APP); ./configure \
		--host=$(TOOLCHAIN_PREFIX) \
		--target=$(TOOLCHAIN_PREFIX) \
		--prefix=/ \
	|| exit 1);
	@echo "$(APP) is configured"
	touch $(APP)/$(CONFIG_COMPLETE_FILE)

conditional_build: $(APP)/$(CONFIG_COMPLETE_FILE)
	$(MAKE) -C $(APP) $(MAKE_ARGS)
	echo "Done building $(APP)"

install:
	@echo Installing to $(INSTALLDIR)
	$(MAKE) -C $(APP) INSTALL_PROGRAM='$$(INSTALL_STRIP_PROGRAM)' STRIPPROG='$$(STRIP)' install-exec

# NOTE: make clean from within app does not do a proper job, so wiping out
# entire directory to ensure consistency.
clean:
	rm -rf $(APP)

# The next line is a hint to our release scripts
# GLOBAL_RELEASE_SCRIPT_CALL_DISTCLEAN
distclean: clean

shell:
	bash

