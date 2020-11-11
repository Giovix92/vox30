SUBDIRS=libsepol libselinux-2.0.85

all:
	@for subdir in $(SUBDIRS); do \
		(cd $$subdir && $(MAKE) $@) || exit 1; \
	done

install:
	install libsepol/src/libsepol.so.1 $(INSTALLDIR)/lib/
	ln -s libsepol.so.1 $(INSTALLDIR)/lib/libsepol.so
	install libselinux-2.0.85/src/libselinux.so.1 $(INSTALLDIR)/lib/
	ln -s libselinux.so.1 $(INSTALLDIR)/lib/libselinux.so

clean:
	@for subdir in $(SUBDIRS); do \
		(cd $$subdir && $(MAKE) $@ clean) || exit 1; \
	done
