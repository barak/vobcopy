#!/bin/sh
#hope this thing works across all systems, otherwise mail me please:
#robos@muon.de


echo "THIS CONFIGURE SCRIPT IS STILL HIGHLY EXPERIMENTAL!
If you think you found a bug, please mail me, thanks! robos@muon.de"

#This is the makefile generator for vobcopy, 
#The original makefile was  written by rosenauer. These things 
#below here are variable definitions. They get substituted in the (CC) and 
#stuff places.


#args check *new*
#declare -i i=0
if [ $# != 0 ]
then 
     while [ $# != 0 ]
     do
       
       if [ "$1" != "${1#--prefix=}" ]; then
       	   prefix="${1#--prefix=}"
	   prefix_provided=true
           echo "prefix = $prefix"
       fi

       if [ "$1" != "${1#--bindir=}" ]; then
       	   bindir="${1#--bindir=}"
	   bindir_provided=true
       	   echo "bindir here: $bindir"
       fi

       if [ "$1" != "${1#--mandir=}" ]; then
       	   mandir="${1#--mandir=}"
	   mandir_provided=true
       	   echo "with mandir here: $mandir"
       fi

       if [ "$1" != "${1#--docdir=}" ]; then
       	   docdir="${1#--docdir=}"
	   docdir_provided=true
       	   echo "with docdir here: $docdir"
       fi

       if [ "$1" != "${1#--with-lfs}" ]; then
  	   echo "with large-file support"
 	   LFS="LFS    = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE"
	   lfs_provided=true
       fi

       if [ "$1" != "${1#--with-dvdread-libs=}" ]; then
       	   libs_dir="${1#--with-dvdread-libs=}"
	   libs_dir_provided=true
       	   echo "with dvdread-libs here: $libs_dir"
       fi
       
       if [ "$1" != "${1%help}" ]; then
	   echo "--prefix=PREFIX                install architecture-independent files in PREFIX [/usr/local]"
	   echo "--bindir=DIR                   user executables in DIR [PREFIX/bin]"
	   echo "--mandir=DIR                   man documentation in DIR [PREFIX/bin]"
	   echo "--docdir=DIR                   package documentation in DIR [PREFIX/share/doc/vobcopy]"
           echo "--with-dvdread-libs=DIR        directory where dvdread lib (dvd_reader.h) is installed"
	   echo "--with-lfs                     Enable large File System support"
           exit 1
       fi
       shift 1
     done
fi

if [ -z $prefix_provided ]
then
     prefix=/usr/local
fi

if [ -z $mandir_provided ]
then
     mandir=\${PREFIX}/man
fi

if [ -z $docdir_provided ]
then
     docdir=\${PREFIX}/share/doc/vobcopy
fi

if [ -z $bindir_provided ]
then
     bindir=\${PREFIX}/bin
fi


#see if libdvdread is installed
if [ -z $libs_dir_provided ]
then

    if [ ! -e /usr/local/include/dvdread ]; then
	if [ ! -e /usr/include/dvdread ]; then #muss hier das then hin??
		echo "Do you have libdvdread installed? I (the script) can't
find it"
		echo "You probably need the libdvdread-devel package or something similar installed. If you already have:"
     		echo "Please provide the path to dvdreader.h after
./configure --with-dvdread-libs="
	    	echo "And please mail me the location so that I can fix the
makefile to robos@muon.de, thanks!"
		exit 1
	else 
	     echo "libdvdread found"
	     libs_dir=/usr/
#	     libs_dir=/usr/include	     
	fi	
    else 
    	 echo "libdvdread found"
	 libs_dir=/usr/local/
#	 libs_dir=/usr/local/include	 
    fi
else
# Remove the following if...fi if the program complains about non-existing
# headers when they really are there...
    if [ ! -e $libs_dir/include/dvdread ]; then
        echo "You specified that libdvdread is installed at $libs_dir. However"
	echo "I (the script) am unable to find the header files at"
	echo "$libs_dir/include/dvdread."
	echo "If they really are there, edit the configure.sh and remove"
	echo "this check."
	exit 1
    fi
fi


#I hope this is a test to see if the system is largefile ready
if [ -z $lfs_provided ]
then
   if grep "ftello64" /usr/include/stdio.h >/dev/null 2>&1
   then
    	echo "largefile support found"
    	LFS="LFS    = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE"	
   else
    	echo "no large-file support found (by me, the stupid script)"
	echo "large file support is necessary if you want to output files 
	larger than 2GB"
	echo "please append --with-lfs to ./configure, else lfs is disabled"
	LFS="LFS    ="
   fi
fi

       
#FreeBSD needs libgnugetopt (kern.osreldate < 500041)
#if [ `uname -s` = FreeBSD -a \
#	`sysctl -n kern.osreldate 2> /dev/null` -lt 500041 ]; then
if [ `uname -s` = FreeBSD ]; then
  if [ `sysctl -n kern.osreldate 2> /dev/null` -lt 500041 ]; then
	  LDFLAGS="LDFLAGS += -ldvdread -L/usr/local/lib -lgnugetopt"
  else
	  LDFLAGS="LDFLAGS += -ldvdread -L$libs_dir/lib"
  fi	
elif [ `uname -m` = x86_64 ]; then #for ia64/AMD64 libraries
	LDFLAGS="LDFLAGS += -ldvdread -L$libs_dir/lib64"
else
	LDFLAGS="LDFLAGS += -ldvdread -L$libs_dir/lib"
fi

#Solaris 9 needs -lrt 
if [ `uname -a | grep "SunOS solaris 5.9" ` ]; then
	LDFLAGS="+LDFLAGS += -ldvdread -lrt -L/usr//lib"
fi



#see if a Makefile is present - and kill it ;-)
if [ -e ./Makefile ]; then
	rm -f ./Makefile
fi

#write the Makefile
touch Makefile

echo "
#This is the makefile for vobcopy, mainly written by rosenauer. These things 
#below here are variable definitions. They get substituted in the (CC) and 
#stuff places.
DESTDIR = 
CC     ?= gcc
#PREFIX += /usr/local
#BINDIR = \${PREFIX}/bin
#MANDIR = \${PREFIX}/man
PREFIX += $prefix
BINDIR = $bindir
MANDIR = $mandir
DOCDIR = $docdir
$LFS
CFLAGS += -I$libs_dir/include
$LDFLAGS

#This specifies the conversion from .c to .o 
.c.o:
	\$(CC) \$(LFS) \$(CFLAGS) -c \$<

#Here is implicitly said that for vobcopy to be made *.o has to be made first
#make is kinda intelligent in that aspect.
vobcopy: vobcopy.o dvd.o 
	\$(CC) -o vobcopy vobcopy.o dvd.o \${LDFLAGS}

disable_lfs:
	\$(CC) \$(CFLAGS) -c vobcopy.c
	\$(CC) \$(CFLAGS) -c dvd.c
	\$(CC) -o vobcopy vobcopy.o dvd.o -ldvdread

debug:
	\$(CC) -c vobcopy.c -Wall -ggdb -pedantic \$(CFLAGS) \$(LFS)
	\$(CC) -c dvd.c     -Wall -ggdb -pedantic \$(CFLAGS) \$(LFS)
	\$(CC) -o vobcopy vobcopy.o dvd.o -ldvdread

deb:
        
	echo \"this here is really really experimental...\"
	dpkg-buildpackage -rfakeroot -us -uc -tc
		

clean :
	rm -f vobcopy vobcopy.o dvd.o

distclean :
	rm -f vobcopy.o dvd.o *~

install:
#	mkdir -p \$(MANDIR)/man1
#	cp vobcopy   \$(BINDIR)/vobcopy
#	cp vobcopy.1 \$(MANDIR)/man1/vobcopy.1
	install -d -m 755 \$(DESTDIR)/\$(BINDIR)
	install -d -m 755 \$(DESTDIR)/\$(MANDIR)/man1
	install -d -m 755 \$(DESTDIR)/\$(MANDIR)/de/man1
	install -d -m 755 \$(DESTDIR)/\$(DOCDIR)
	install -m 755 vobcopy \$(DESTDIR)/\$(BINDIR)/vobcopy
	install -m 644 vobcopy.1 \$(DESTDIR)/\$(MANDIR)/man1/vobcopy.1
	install -m 644 vobcopy.1.de \$(DESTDIR)/\$(MANDIR)/de/man1/vobcopy.1
	install -m 644 COPYING Changelog README Release-Notes TODO \$(DESTDIR)/\$(DOCDIR)

uninstall:
	rm -f \$(DESTDIR)/\$(BINDIR)/vobcopy
	rm -f \$(DESTDIR)/\$(MANDIR)/man1/vobcopy.1
	rm -f \$(DESTDIR)/\$(MANDIR)/de/man1/vobcopy.1	
	rm -f \$(DESTDIR)/\$(DOCDIR)/{COPYING,Changelog,README,Release-Notes,TODO}
	rmdir --parents \$(DESTDIR)/\$(BINDIR) 2>/dev/null
	rmdir --parents \$(DESTDIR)/\$(MANDIR)/man1 2>/dev/null
	rmdir --parents \$(DESTDIR)/\$(MANDIR)/de/man1 2>/dev/null
	rmdir --parents \$(DESTDIR)/\$(DOCDIR) 2>/dev/null
" > Makefile

echo "Next thing type \"make\" and then \"make install\""
