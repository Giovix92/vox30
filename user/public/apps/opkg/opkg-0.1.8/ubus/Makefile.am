AM_CFLAGS = -I${top_srcdir}/libopkg ${ALL_CFLAGS} -I$(JSON_DIR) -I$(UBOX_DIR)/include/ -I$(UBUS_DIR) 
bin_PROGRAMS = opkg-ubus

opkg_cl_SOURCES = opkg-ubus.c
opkg_cl_LDADD = $(top_builddir)/libopkg/libopkg.la \
                $(top_builddir)/libbb/libbb.la 
LDFLAGS = -L$(INSTALLDIR)/lib -lblobmsg_json -ljson-c -lubox -lubus -L$(top_builddir)/libopkg/.libs/ -lopkg

