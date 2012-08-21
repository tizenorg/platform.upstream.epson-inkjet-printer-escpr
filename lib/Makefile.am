################################################################
# Protocol set for esc/p-r lib
################################################################
DEFINES			=
DEFINES 		+=	-DGCOMSW_PRT_USE_USB
DEFINES 		+=	-DGCOMSW_PRT_USE_LPR
DEFINES 		+=	-DGCOMSW_PRT_USE_RAW

################################################################
# Extend function
################################################################
DEFINES 		+=	-DGCOMSW_CMD_ESCPAGE
DEFINES 		+=	-DGCOMSW_EL_CDLABEL
DEFINES 		+=	-DGCOMSW_EF_MAINTE

################################################################
# Language
################################################################
DEFINES 		+=	-DGCOMSW_CMD_ESCPAGE
DEFINES 		+= 	-DEPS_FILTER

################################################################
# Use Async job process
DEFINES 		+=	-DGCOMSW_ASYNCJOB_SUPPORT
# Use raw file print
DEFINES 	    +=	-DGCOMSW_RAWFILE_SUPPORT


lib_LTLIBRARIES = libescpr.la
libescpr_la_CFLAGS = -Wall $(DEFINES)
## Make sure these will be cleaned even when they're not built by
## default.
CLEANFILES = libescpr.la
libescpr_la_SOURCES = \
	epson-cbt.c epson-cbt.h \
	epson-escpage.c epson-escpage.h \
	epson-escpage-color.c epson-escpage-color.h \
	epson-escpage-comp.c epson-escpage-comp.h \
	epson-escpage-mono.c epson-escpage-mono.h \
	epson-escpr-api.c epson-escpr-api.h \
	epson-escpr-dbg.c epson-escpr-dbg.h \
	epson-escpr-def.h \
	epson-escpr-err.h \
	epson-escpr-media.h \
	epson-escpr-mem.c epson-escpr-mem.h \
	epson-escpr-pvt.h \
	epson-escpr-services.c epson-escpr-services.h \
	epson-layout.c epson-layout.h \
	epson-net-lpr.c epson-net-lpr.h \
	epson-net-raw.c epson-net-raw.h \
	epson-net-snmp.c epson-net-snmp.h \
	epson-protocol.c epson-protocol.h \
	epson-typedefs.h \
	epson-usb.c epson-usb.h \
	escpr_def.h \
	escpr_osdep.h 
libescpr_la_LDFLAGS = -O2 -version-info 1:0:0

