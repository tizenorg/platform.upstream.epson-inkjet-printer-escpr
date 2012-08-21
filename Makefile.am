SUBDIRS = \
	layout_script \
	lib \
	ppd \
	src

ACLOCAL_AMFLAGS = -I m4

noinst_DATA = \
	debian/control \
	debian/rules

debian/control: $(srcdir)/Makefile $(srcdir)/debian/control.in
	rm -f $@ $@.tmp; \
	sed -e 's|[@]PACKAGE_BUGREPORT[@]|$(PACKAGE_BUGREPORT)|' \
	    -e 's|[@]PACKAGE_NAME[@]|$(PACKAGE_NAME)|' \
	    -e 's|[@]PACKAGE_TARNAME[@]|$(PACKAGE_TARNAME)|' \
	    '$(top_srcdir)/$@.in' > $@.tmp; \
	mv $@.tmp $@

debian/rules: $(srcdir)/Makefile $(srcdir)/debian/rules.in
	rm -f $@ $@.tmp; \
	sed -e 's|[@]CUPS_FILTER_DIR[@]|$(CUPS_FILTER_DIR)|' \
	    -e 's|[@]CUPS_PPD_DIR[@]|$(CUPS_PPD_DIR)|' \
	    -e 's|[@]PACKAGE_TARNAME[@]|$(PACKAGE_TARNAME)|' \
	    '$(top_srcdir)/$@.in' > $@.tmp; \
	mv $@.tmp $@; \
	chmod +x $@

#  Automate maintenance of the list of shipped M4 macros.
#  Do not change the list unnecessarily.  As a side effect, this also
#  works around issues with a read-only $(srcdir) during a 'distcheck'.

$(top_builddir)/m4-macros: $(top_srcdir)/m4
	echo M4_MACROS = `ls $^ | sed 's,^,m4/,'` > m4-macros.tmp
	cmp -s m4-macros.tmp $@ || cp m4-macros.tmp $@
	rm m4-macros.tmp

include m4-macros


EXTRA_DIST = \
	$(M4_MACROS) \
	$(noinst_DATA) \
	AUTHORS \
	COPYING \
	NEWS \
	README \
	README.ja \
	bootstrap \
	debian/changelog \
	debian/compat \
	debian/control.in \
	debian/copyright \
	debian/docs \
	debian/postinst \
	debian/postrm \
	debian/rules.in \
	epson-inkjet-printer-escpr.spec \
	epson-inkjet-printer-escpr.spec.in \
	lsb/lsb-rpm.spec \
	lsb/lsb-rpm.spec.in \
	m4
