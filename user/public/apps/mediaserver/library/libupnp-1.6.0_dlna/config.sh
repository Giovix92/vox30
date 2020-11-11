#!/bin/sh
# $Header: /home/cvsadmin/sbu-i/HGI/DLNA_new/mediaserver/library/libupnp-1.6.0_dlna/config.sh,v 1.1.1.1 2009-05-05 05:07:01 gavin_zhang Exp $

# config.sh
# Created at Mon 09 Oct 2006 01:51:38 PM UTC by jick.
# 

./configure \
--host=arm-linux-gnueabi \
--prefix=/usr \
--libdir=/lib \
--disable-libtool-lock \
--disable-nls \
--disable-rpath \
$1
