# epson-inkjet-printer-escpr.spec.in -- an rpm spec file templete
# Epson Inkjet Printer Driver (ESC/P-R) for Linux
# Copyright (C) 2000-2006 AVASYS CORPORATION.
# Copyright (C) Seiko Epson Corporation 2006-2012.
#  This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA.

%define _unpackaged_files_terminate_build 0
%define cupsfilterdir   /usr/lib/cups/filter
%define cupsppddir      /opt/etc/cups/ppd/Epson

Name: epson-inkjet-printer-escpr
Version: 1.2.0
Release: 1
Source0: %{name}-%{version}.tar.gz
License: GPL
Vendor: Seiko Epson Corporation
URL: http://download.ebz.epson.net/dsc/search/01/search/?OSC=LX
Packager: Seiko Epson Corporation <linux-printer@epson.jp>
BuildRoot: %{_tmppath}/%{name}-%{version}
BuildRequires: cups-devel
Group: Applications/System
Summary: Epson Inkjet Printer Driver (ESC/P-R) for Linux

%description
This software is a filter program used with Common UNIX Printing
System (CUPS) from the Linux. This can supply the high quality print
with Seiko Epson Color Ink Jet Printers.

This product supports only EPSON ESC/P-R printers. This package can be
used for all EPSON ESC/P-R printers.

For detail list of supported printer, please refer to below site:
http://download.ebz.epson.net/dsc/search/01/search/?OSC=LX

%prep
%setup -q

%build
%configure \
	--with-cupsfilterdir=%{cupsfilterdir} \
	--with-cupsppddir=%{cupsppddir} \
	--disable-static
make

%install
rm -rf ${RPM_BUILD_ROOT}
make install-strip DESTDIR=${RPM_BUILD_ROOT}

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%clean
make clean
rm -rf ${RPM_BUILD_ROOT}

%files
#%manifest epson-inkjet-printer-escpr.manifest
%defattr(-,root,root)
%doc AUTHORS COPYING NEWS README README.ja
%{cupsfilterdir}/epson-escpr
%{cupsfilterdir}/epson-escpr-wrapper
%{_libdir}/libescpr.so*
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/paper_list.csv
#%{cupsppddir}/*.ppd
