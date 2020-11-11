#!/bin/sh
case "$PLUTO_VERB" in
    up-host)
    killall openl2tpd
    openl2tpd
    ;;
    down-host)
    killall pppd_30
    ;;
esac
