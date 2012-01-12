bin_PROGRAMS = vobcopy

vobcopy_SOURCES = dvd.c vobcopy.c dvd.h vobcopy.h

AM_CPPFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE

dist_man_MANS = vobcopy.1
man1dedir = $(mandir)/de/man1
man1de_DATA = intl/de/vobcopy.1

EXTRA_DIST = vobcopy.mo vobcopy.pot vobcopy.spec vobcopy.1.de

dist_doc_DATA = alternative_programs.txt Changelog README Release-Notes TODO COPYING

intl/de/vobcopy.1: vobcopy.1.de
	mkdir -p intl/de
	ln $< $@

MOSTLYCLEANFILES = intl/de/vobcopy.1

mostlyclean-local:
	-rmdir intl/de
	-rmdir intl
