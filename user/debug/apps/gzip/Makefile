all:
ifneq ($(INSTALLDIR),"")
ifeq ($(GPL), 1)
	./build.sh
else
	echo "CP the gzip to \"$INSTALLDIR/bin/\""
ifeq ($(PRODUCT_TYPE), VOX30)
ifeq ($(BRCM_502), 1)
	cp -af ./gzip_arm.502 $(INSTALLDIR)/bin/gzip
else
	cp -af ./gzip_arm $(INSTALLDIR)/bin/gzip
endif
else
	cp -af ./gzip $(INSTALLDIR)/bin
endif
endif
endif
	# recompile tcpdump
	#./build.sh 

clean:
install:

