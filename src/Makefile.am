cupsfilterdir = $(CUPS_FILTER_DIR)
ESCPR_LIB_DIR	=	../escprlib

cupsfilter_PROGRAMS =									\
	epson-escpr												\
	epson-escpr-wrapper

epson_escpr_wrapper_CFLAGS =							\
	-DCUPS_FILTER_NAME=\"epson-escpr\"				\
	-DCUPS_FILTER_PATH=\"/usr/lib/cups/filter\" 

epson_escpr_wrapper_SOURCES =							\
	wrapper.c

epson_escpr_CFLAGS =										\
	-I$(top_srcdir)/lib									\
	-DPAPER_PATH=\"$(pkgdatadir)/paper_list.csv\" 

epson_escpr_SOURCES =									\
	filter.c													\
	libprtX.h												\
	debug.c debug.h										\
	pips.h													\
	pipsDef.h												\
	optBase.h												\
	str.c str.h												\
	err.c err.h												\
	csv.c csv.h												\
	mem.c mem.h												\
	linux_cmn.h linux_cmn.c								\
	def.h

epson_escpr_LDADD = \
	$(top_builddir)/lib/libescpr.la

