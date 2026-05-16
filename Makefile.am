bin_PROGRAMS = vobcopy

vobcopy_SOURCES = dvd.c vobcopy.c dvd.h vobcopy.h

dist_man_MANS = vobcopy.1
man1dedir = $(mandir)/de/man1
man1de_DATA = intl/de/vobcopy.1

check_PROGRAMS = tests/unit_helpers
TESTS = tests/cli-smoke.sh $(check_PROGRAMS)
TEST_EXTENSIONS = .sh
SH_LOG_COMPILER = $(SHELL)
AM_TESTS_ENVIRONMENT = top_builddir='$(top_builddir)'; export top_builddir;

tests_unit_helpers_SOURCES = tests/unit_helpers.c
tests_unit_helpers_LDADD = -ldvdread

EXTRA_DIST = vobcopy.spec vobcopy.1.de tests/cli-smoke.sh tests/unit_helpers.c

dist_doc_DATA = alternative_programs.txt Changelog README Release-Notes TODO COPYING

intl/de/vobcopy.1: vobcopy.1.de
	$(MKDIR_P) intl/de
	ln $< $@

MOSTLYCLEANFILES = intl/de/vobcopy.1

mostlyclean-local:
	-rmdir intl/de
	-rmdir intl
