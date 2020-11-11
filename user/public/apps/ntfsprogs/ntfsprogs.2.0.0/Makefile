HAVE_SC_CACHE=y
export HAVE_SC_CACHE

all:
	$(MAKE) -C libntfs
	$(MAKE) -C ntfsprogs

install:
	$(MAKE) -C libntfs install
	$(MAKE) -C ntfsprogs install

clean:
	$(MAKE) -C libntfs clean
	$(MAKE) -C ntfsprogs clean

prepare:
	@echo "fuse prepare nothing"

GPL:
	cp -a $(shell pwd) $(GPL_DIR)/common/
