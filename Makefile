
#This is the makefile for vobcopy, mainly written by rosenauer. These things 
#below here are variable definitions. They get substituted in the (CC) and 
#stuff places.
DESTDIR = 
CC     ?= gcc
#PREFIX += /usr/local
#BINDIR = ${PREFIX}/bin
#MANDIR = ${PREFIX}/man
PREFIX += /usr/local
BINDIR = ${PREFIX}/bin
MANDIR = ${PREFIX}/man
DOCDIR = ${PREFIX}/share/doc/vobcopy
LFS    = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
CFLAGS += -I/usr/local//include
LDFLAGS += -ldvdread -L/usr/local//lib

#This specifies the conversion from .c to .o 
.c.o:
	$(CC) $(LFS) $(CFLAGS) -c $<

#Here is implicitly said that for vobcopy to be made *.o has to be made first
#make is kinda intelligent in that aspect.
vobcopy: vobcopy.o dvd.o 
	$(CC) -o vobcopy vobcopy.o dvd.o ${LDFLAGS}

disable_lfs:
	$(CC) $(CFLAGS) -c vobcopy.c
	$(CC) $(CFLAGS) -c dvd.c
	$(CC) -o vobcopy vobcopy.o dvd.o -ldvdread

debug:
	$(CC) -c vobcopy.c -Wall -ggdb -pedantic $(CFLAGS) $(LFS)
	$(CC) -c dvd.c     -Wall -ggdb -pedantic $(CFLAGS) $(LFS)
	$(CC) -o vobcopy vobcopy.o dvd.o -ldvdread

deb:
        
	echo "this here is really really experimental..."
	dpkg-buildpackage -rfakeroot -us -uc -tc
		

clean :
	rm -f vobcopy vobcopy.o dvd.o

distclean :
	rm -f vobcopy.o dvd.o *~

install:
#	mkdir -p $(MANDIR)/man1
#	cp vobcopy   $(BINDIR)/vobcopy
#	cp vobcopy.1 $(MANDIR)/man1/vobcopy.1
	install -d -m 755 $(DESTDIR)/$(BINDIR)
	install -d -m 755 $(DESTDIR)/$(MANDIR)/man1
	install -d -m 755 $(DESTDIR)/$(MANDIR)/de/man1
	install -d -m 755 $(DESTDIR)/$(DOCDIR)
	install -m 755 vobcopy $(DESTDIR)/$(BINDIR)/vobcopy
	install -m 644 vobcopy.1 $(DESTDIR)/$(MANDIR)/man1/vobcopy.1
	install -m 644 vobcopy.1.de $(DESTDIR)/$(MANDIR)/de/man1/vobcopy.1
	install -m 644 COPYING Changelog README Release-Notes TODO $(DESTDIR)/$(DOCDIR)

uninstall:
	rm -f $(DESTDIR)/$(BINDIR)/vobcopy
	rm -f $(DESTDIR)/$(MANDIR)/man1/vobcopy.1
	rm -f $(DESTDIR)/$(MANDIR)/de/man1/vobcopy.1	
	rm -f $(DESTDIR)/$(DOCDIR)/{COPYING,Changelog,README,Release-Notes,TODO}
	rmdir --parents $(DESTDIR)/$(BINDIR) 2>/dev/null
	rmdir --parents $(DESTDIR)/$(MANDIR)/man1 2>/dev/null
	rmdir --parents $(DESTDIR)/$(MANDIR)/de/man1 2>/dev/null
	rmdir --parents $(DESTDIR)/$(DOCDIR) 2>/dev/null

