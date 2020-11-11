#!/bin/sh


cd ./gzip124 && make
cd -

# install
if [ ! -z $INSTALLDIR ]; then
	echo "CP the gzip to \"$INSTALLDIR/bin/\""
	$STRIP ./gzip124/gzip
	install -D ./gzip124/gzip $INSTALLDIR/bin
fi

