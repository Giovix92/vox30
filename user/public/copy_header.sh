#!/bin/sh
mkdir $1/public
mkdir $1/public/include

mkdir $1/public/include/log
cp $PWD/user/public/include/log/*.h $1/public/include/log/
mkdir $1/public/include/lua
cp $PWD/user/public/include/lua/*.h $1/public/include/lua/
mkdir $1/public/include/readline
cp $PWD/user/public/include/readline/*.h $1/public/include/readline/

mkdir $1/public/include/curl
cp $PWD/user/private/libs/libcurl/curl-7.47.0/include/curl/*.h $1/public/include/curl/

mkdir $1/public/libs

mkdir $1/public/libs/fuse
mkdir $1/public/libs/fuse/fuse.2.7.4/
mkdir $1/public/libs/fuse/fuse.2.7.4/include
mkdir $1/public/libs/fuse/fuse.2.7.4/lib/
cp $PWD/user/public/libs/fuse/fuse.2.7.4/include/*.h $1/public/libs/fuse/fuse.2.7.4/include/
cp $PWD/user/public/libs/fuse/fuse.2.7.4/lib/*.h $1/public/libs/fuse/fuse.2.7.4/lib/


mkdir $1/public/libs/libapparmor
mkdir $1/public/libs/libapparmor/apparmor-2.12
mkdir $1/public/libs/libapparmor/apparmor-2.12/libraries
mkdir $1/public/libs/libapparmor/apparmor-2.12/libraries/libapparmor
mkdir $1/public/libs/libapparmor/apparmor-2.12/libraries/libapparmor/include/
cp $PWD/user/public/libs/libapparmor/apparmor-2.12/libraries/libapparmor/include/*.h $1/public/libs/libapparmor/apparmor-2.12/libraries/libapparmor/include/
mkdir $1/public/libs/libapparmor/apparmor-2.12/libraries/libapparmor/include/sys
cp $PWD/user/public/libs/libapparmor/apparmor-2.12/libraries/libapparmor/include/sys/*.h $1/public/libs/libapparmor/apparmor-2.12/libraries/libapparmor/include/sys/

mkdir $1/public/libs/libcap
mkdir $1/public/libs/libcap/libcap-2.25
mkdir $1/public/libs/libcap/libcap-2.25/libcap
mkdir $1/public/libs/libcap/libcap-2.25/libcap/include
cp -r $PWD/user/public/libs/libcap/libcap-2.25/libcap/include/* $1/public/libs/libcap/libcap-2.25/libcap/include/


mkdir $1/public/libs/libflex
cp -r $PWD/user/public/libs/libflex/*.h $1/public/libs/libflex/

mkdir $1/public/libs/libgmp
mkdir $1/public/libs/libgmp/gmp-6.1.2
cp -r $PWD/user/public/libs/libgmp/gmp-6.1.2/*.h $1/public/libs/libgmp/gmp-6.1.2/
mkdir $1/public/libs/libgmp/gmp-6.1.2/mpn
mkdir $1/public/libs/libgmp/gmp-6.1.2/mpn/arm
cp -r $PWD/user/public/libs/libgmp/gmp-6.1.2/mpn/arm/*.h $1/public/libs/libgmp/gmp-6.1.2/mpn/arm/

mkdir $1/public/libs/libgpg-error
mkdir $1/public/libs/libgpg-error/libgpg-error-1.32
mkdir $1/public/libs/libgpg-error/libgpg-error-1.32/src
cp -r $PWD/user/public/libs/libgpg-error/libgpg-error-1.32/src/*.h $1/public/libs/libgpg-error/libgpg-error-1.32/src/
mkdir $1/public/libs/libgpg-error/libgpg-error-1.32/src/syscfg
cp -r $PWD/user/public/libs/libgpg-error/libgpg-error-1.32/src/syscfg/*.h $1/public/libs/libgpg-error/libgpg-error-1.32/src/syscfg/

mkdir $1/public/libs/libgcrypt
mkdir $1/public/libs/libgcrypt/libgcrypt-1.8.3
mkdir $1/public/libs/libgcrypt/libgcrypt-1.8.3/src
cp -r $PWD/user/public/libs/libgcrypt/libgcrypt-1.8.3/src/*.h $1/public/libs/libgcrypt/libgcrypt-1.8.3/src/
mkdir $1/public/libs/libgcrypt/libgcrypt-1.8.3/cipher
cp -r $PWD/user/public/libs/libgcrypt/libgcrypt-1.8.3/cipher/*.h $1/public/libs/libgcrypt/libgcrypt-1.8.3/cipher/
mkdir $1/public/libs/libgcrypt/libgcrypt-1.8.3/random
cp -r $PWD/user/public/libs/libgcrypt/libgcrypt-1.8.3/random/*.h $1/public/libs/libgcrypt/libgcrypt-1.8.3/random/
mkdir $1/public/libs/libgcrypt/libgcrypt-1.8.3/mpi
cp -r $PWD/user/public/libs/libgcrypt/libgcrypt-1.8.3/mpi/*.h $1/public/libs/libgcrypt/libgcrypt-1.8.3/mpi/
mkdir $1/public/libs/libgcrypt/libgcrypt-1.8.3/mpi/arm
cp -r $PWD/user/public/libs/libgcrypt/libgcrypt-1.8.3/mpi/arm/*.h $1/public/libs/libgcrypt/libgcrypt-1.8.3/mpi/arm/
mkdir $1/public/libs/libgcrypt/libgcrypt-1.8.3/compat
cp -r $PWD/user/public/libs/libgcrypt/libgcrypt-1.8.3/compat/*.h $1/public/libs/libgcrypt/libgcrypt-1.8.3/compat/

mkdir $1/public/libs/libgnutls
mkdir $1/public/libs/libgnutls/gnutls-3.6.2
mkdir $1/public/libs/libgnutls/gnutls-3.6.2/lib
mkdir $1/public/libs/libgnutls/gnutls-3.6.2/lib/includes
mkdir $1/public/libs/libgnutls/gnutls-3.6.2/lib/includes/gnutls
cp $PWD/user/public/libs/libgnutls/gnutls-3.6.2/lib/includes/gnutls/*.h $1/public/libs/libgnutls/gnutls-3.6.2/lib/includes/gnutls/

mkdir $1/public/libs/libiconv
mkdir $1/public/libs/libiconv/libiconv.1.8
mkdir $1/public/libs/libiconv/libiconv.1.8/include
cp -r $PWD/user/public/libs/libiconv/libiconv.1.8/include/*.h $1/public/libs/libiconv/libiconv.1.8/include/

mkdir $1/public/libs/libmicrohttpd
mkdir $1/public/libs/libmicrohttpd/libmicrohttpd-0.9.59
mkdir $1/public/libs/libmicrohttpd/libmicrohttpd-0.9.59/src
mkdir $1/public/libs/libmicrohttpd/libmicrohttpd-0.9.59/src/include/
cp -r $PWD/user/public/libs/libmicrohttpd/libmicrohttpd-0.9.59/MHD_config.h $1/public/libs/libmicrohttpd/libmicrohttpd-0.9.59/MHD_config.h
cp -r $PWD/user/public/libs/libmicrohttpd/libmicrohttpd-0.9.59/src/include/*.h $1/public/libs/libmicrohttpd/libmicrohttpd-0.9.59/src/include/
mkdir $1/public/libs/libmicrohttpd/libmicrohttpd-0.9.59/src/microhttpd/
cp -r $PWD/user/public/libs/libmicrohttpd/libmicrohttpd-0.9.59/src/microhttpd/*.h $1/public/libs/libmicrohttpd/libmicrohttpd-0.9.59/src/microhttpd/
mkdir $1/public/libs/libmosquitto
mkdir $1/public/libs/libmosquitto/mosquitto-master
mkdir $1/public/libs/libmosquitto/mosquitto-master/lib
mkdir $1/public/libs/libmosquitto/mosquitto-master/src
cp $PWD/user/public/libs/libmosquitto/mosquitto-master/lib/*.h $1/public/libs/libmosquitto/mosquitto-master/lib/
cp $PWD/user/public/libs/libmosquitto/mosquitto-master/src/*.h $1/public/libs/libmosquitto/mosquitto-master/src/
mkdir $1/public/libs/libncurses
mkdir $1/public/libs/libncurses/ncurses-6.1
mkdir $1/public/libs/libncurses/ncurses-6.1/include
cp $PWD/user/public/libs/libncurses/ncurses-6.1/include/*.h $1/public/libs/libncurses/ncurses-6.1/include/
mkdir $1/public/libs/libnetlink
cp -r $PWD/user/public/libs/libnetlink/include $1/public/libs/libnetlink/
mkdir $1/public/libs/libnettle/
mkdir $1/public/libs/libnettle/nettle
cp -r $PWD/user/public/libs/libnettle/nettle/*.h $1/public/libs/libnettle/nettle/
mkdir $1/public/libs/libnfnetlink
mkdir $1/public/libs/libnfnetlink/include
mkdir $1/public/libs/libnfnetlink/include/libnfnetlink
cp -r $PWD/user/public/libs/libnfnetlink/include/libnfnetlink/*.h $1/public/libs/libnfnetlink/include/libnfnetlink/
mkdir $1/public/libs/libreadline
mkdir $1/public/libs/libreadline/readline-5.2
cp -r $PWD/user/public/libs/libreadline/readline-5.2/*.h $1/public/libs/libreadline/readline-5.2/
mkdir $1/public/libs/libseccomp
mkdir $1/public/libs/libseccomp/libseccomp-2.1.1
mkdir $1/public/libs/libseccomp/libseccomp-2.1.1/include
cp -r $PWD/user/public/libs/libseccomp/libseccomp-2.1.1/include/*.h $1/public/libs/libseccomp/libseccomp-2.1.1/include/
mkdir $1/public/libs/libselinux
mkdir $1/public/libs/libselinux/libselinux-2.0.85
mkdir $1/public/libs/libselinux/libselinux-2.0.85/include
mkdir $1/public/libs/libselinux/libselinux-2.0.85/include/selinux
cp $PWD/user/public/libs/libselinux/libselinux-2.0.85/include/selinux/*.h $1/public/libs/libselinux/libselinux-2.0.85/include/selinux/
mkdir $1/public/libs/libsrv
mkdir $1/public/libs/libsrv/include
mkdir $1/public/libs/libsrv/include/arpa
cp $PWD/user/public/libs/libsrv/include/*.h $1/public/libs/libsrv/include/
cp $PWD/user/public/libs/libsrv/include/arpa/*.h $1/public/libs/libsrv/include/arpa/
mkdir $1/public/libs/libusb
mkdir $1/public/libs/libusb/libusb-1.0.9
mkdir $1/public/libs/libusb/libusb-1.0.9/libusb
cp -r $PWD/user/public/libs/libusb/libusb-1.0.9/libusb/libusb.h $1/public/libs/libusb/libusb-1.0.9/libusb/
cp -r $PWD/user/public/libs/libusb/libusb-1.0.9/libusb/libusbi.h $1/public/libs/libusb/libusb-1.0.9/libusb/
mkdir $1/public/libs/openssl
mkdir $1/public/libs/openssl/openssl-1.0.2q
mkdir -p $1/public/libs/openssl/openssl-1.0.2q/include/openssl
cp -f $PWD/user/public/libs/openssl/openssl-1.0.2q/include/openssl/* $1/public/libs/openssl/openssl-1.0.2q/include/openssl
mkdir $1/public/libs/pcre/
mkdir $1/public/libs/pcre/pcre-8.42/
cp -r $PWD/user/public/libs/pcre/pcre-8.42/*.h $1/public/libs/pcre/pcre-8.42/
mkdir $1/public/libs/python
mkdir $1/public/libs/python/Python-3.5.2
mkdir $1/public/libs/python/Python-3.5.2/Include
cp -r $PWD/user/public/libs/python/Python-3.5.2/Include/*.h $1/public/libs/python/Python-3.5.2/Include/
mkdir $1/public/libs/ubus
mkdir $1/public/libs/ubus/json-c
mkdir $1/public/libs/ubus/json-c/json-c
cp -r $PWD/user/public/libs/ubus/json-c/json-c/*.h $1/public/libs/ubus/json-c/json-c/
mkdir $1/public/libs/ubus/libubox
mkdir $1/public/libs/ubus/libubox/libubox
cp -r $PWD/user/public/libs/ubus/libubox/libubox/include $1/public/libs/ubus/libubox/libubox/
mkdir $1/public/libs/ubus/ubus
mkdir $1/public/libs/ubus/ubus/ubus
cp -r $PWD/user/public/libs/ubus/ubus/ubus/*.h $1/public/libs/ubus/ubus/ubus/
mkdir $1/public/libs/uci
mkdir $1/public/libs/uci/uci-01012018
cp -r $PWD/user/public/libs/uci/uci-01012018/*.h $1/public/libs/uci/uci-01012018/
mkdir $1/public/libs/zlib
mkdir $1/public/libs/zlib/zlib-1.2.11
cp -r $PWD/user/public/libs/zlib/zlib-1.2.11/*.h $1/public/libs/zlib/zlib-1.2.11/
mkdir $1/public/apps
mkdir $1/public/apps/lxc
mkdir $1/public/apps/lxc/lxc-1.0.6
mkdir $1/public/apps/lxc/lxc-1.0.6/src
mkdir $1/public/apps/lxc/lxc-1.0.6/src/lxc
cp -r $PWD/user/public/apps/lxc/lxc-1.0.6/src/lxc/*.h $1/public/apps/lxc/lxc-1.0.6/src/lxc/
mkdir $1/public/apps/opkg
mkdir $1/public/apps/opkg/opkg-0.1.8
mkdir $1/public/apps/opkg/opkg-0.1.8/libopkg
mkdir $1/public/apps/opkg/opkg-0.1.8/libbb
cp -r $PWD/user/public/apps/opkg/opkg-0.1.8/libopkg/*.h $1/public/apps/opkg/opkg-0.1.8/libopkg/
cp -r $PWD/user/public/apps/opkg/opkg-0.1.8/libbb/*.h $1/public/apps/opkg/opkg-0.1.8/libbb/
find $1/public/ -name .svn | xargs rm -rf 
