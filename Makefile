
#This is the makefile for vobcopy, mainly written by rosenauer. These things 
#below here are variable definitions. They get substituted in the (CC) and 
#stuff places.
CC     ?= gcc
#PREFIX += /usr/local
#BINDIR = ${PREFIX}/bin
#MANDIR = ${PREFIX}/man
PREFIX += /usr/local
BINDIR = ${PREFIX}/bin
MANDIR = ${PREFIX}/man
LFS    = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
CFLAGS += -I/usr//include
LDFLAGS += -ldvdread -L/usr//lib

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
	install -d -m 755 $(BINDIR)
	install -d -m 755 $(MANDIR)/man1
	install -d -m 755 $(MANDIR)/de/man1
	install -m 755 vobcopy $(BINDIR)/vobcopy
	install -m 644 vobcopy.1 $(MANDIR)/man1/vobcopy.1
	install -m 644 vobcopy.1.de $(MANDIR)/de/man1/vobcopy.1

uninstall:
	rm -f $(BINDIR)/vobcopy
	rm -f $(MANDIR)/man1/vobcopy.1
	rm -f $(MANDIR)/de/man1/vobcopy.1	

