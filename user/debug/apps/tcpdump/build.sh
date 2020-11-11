#!/bin/sh

# libpcap
if [ ! -f "./libpcap-1.0.0/Makefile" ]; then
	echo "Create Makefile for libpcap"
	cd ./libpcap-1.0.0 &&  ./configure CC=$CC --target="mips-target-linux-gnu" --host="mips-target-linux-gnu"
	cd -
fi
cd ./libpcap-1.0.0 && make
cd -

# tcpdump
if [ ! -f "./tcpdump-4.0.0/Makefile" ]; then
	echo "Create Makefile for tcpdump"
	cd ./tcpdump-4.0.0 &&  ./configure CC=$CC --target="mips-target-linux-gnu" --host="mips-target-linux-gnu"
	cd -
fi
cd ./tcpdump-4.0.0 && make
cd -

# install
if [ ! -z $INSTALLDIR ]; then
	echo "CP the tcpdump to \"$INSTALLDIR/usr/sbin/\""
	$STRIP ./tcpdump-4.0.0/tcpdump
	install -D ./tcpdump-4.0.0/tcpdump $INSTALLDIR/usr/sbin/
fi

