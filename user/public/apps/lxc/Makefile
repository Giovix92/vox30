APP = lxc-1.0.6
LIB=liblxc.so.1.0.6
all:
	if [ ! -e $(APP)/Makefile ]; then \
	cd $(APP) && ./autogen.sh && ./configure --host="arm" --prefix=/tmp/lxc CC=${CC} --enable-doc=no --enable-api-docs=no --enable-examples=no --enable-bash=no --with-distro=no --enable-seccomp=no with_runtime_path=/tmp/run/; \
	fi
	@echo "Making $(APP)"
	cd $(APP); \
	make; 

install:
	install -m 755 $(APP)/src/lxc/lxc-create $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-start $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-stop $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-cgroup $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-destroy $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-attach $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-freeze $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-info $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-ls $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-unfreeze $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-autostart $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-checkconfig $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-clone $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-config $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-console $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-device $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-execute $(INSTALLDIR)/bin/
	install -m 644 $(APP)/src/lxc/liblxc.so $(INSTALLDIR)/lib/$(LIB)
	install -m 755 $(APP)/src/lxc/lxc-monitor $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-monitord $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-snapshot $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-unshare $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-user-nic $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-usernsexec $(INSTALLDIR)/bin/
	install -m 755 $(APP)/src/lxc/lxc-wait $(INSTALLDIR)/bin/
	cd $(INSTALLDIR)/lib/; \
	ln -sf $(LIB) liblxc.so.1; \
	ln -sf liblxc.so.1 liblxc.so 

clean:
	@if [ -e $(APP)/Makefile ]; then \
		cd $(APP); $(MAKE) clean ; \
	fi
