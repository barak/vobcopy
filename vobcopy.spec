Name: vobcopy
Summary: vobcopy copies DVD .vob files to harddisk
Version: 1.2.0
Release: 1
License: GPL
Group: Applications/Multimedia
URL: http://vobcopy.org/projects/c/c.shtml
Source0: %{name}-%{version}.tar.gz
Buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root
Requires: libdvdread
BuildPrereq: libdvdread-devel

%description
vobcopy called without arguments will find the mounted dvd and copy the
title with the most chapters to the current working directory (thats the
directory you're invoking vobcopy from). It will merge together the
sub-vobs of each title-vob (vts_xx_yy.vob => the xx is the title-vob,
the yy and friends are the sub-vobs, mostly of 1 GB size) and copy them
to harddisk in 2 GB chunks. It will get the title of the movie from the
dvd and copy the data to name-of-moviexx-1.vob, name-of-moviexx-2.vob
(the xx being the title number). Also possible is to mirror the whole video
dvd content and single files can also be copied.

%prep 
%setup -q

%build 
export CFLAGS="$RPM_OPT_FLAGS"
make

%install 
rm -fr %{buildroot}
install -d -m 0755 %{buildroot}/%{_bindir}
install -d -m 0755 %{buildroot}/%{_mandir}/man1
install -d -m 0755 %{buildroot}/%{_mandir}/de
install -d -m 0755 %{buildroot}/%{_mandir}/de/man1
install -m 0755 vobcopy %{buildroot}/%{_bindir}/
install -m 0644 vobcopy.1 %{buildroot}/%{_mandir}/man1/
install -m 0644 vobcopy.1.de %{buildroot}/%{_mandir}/de/man1/vobcopy.1

%clean 
rm -fr %{buildroot}

%files 
%defattr(-,root,root) 
%{_bindir}/* 
%{_mandir}/* 
%doc Changelog COPYING README Release-Notes TODO alternative_programs.txt

%changelog 
* Mon Jun 8 2009 Robos <robos@muon.de>
- 1.2.0: - guess what - see changelog :-)

* Wed Oct 8 2008 Robos <robos@muon.de>
- 1.1.2: - guess what - see changelog :-)

* Sun Mar 3 2008 Robos <robos@muon.de>
- 1.1.1: -see changelog

* Sun Jan 13 2008 Robos  <robos@muon.de>
- 1.1.0: -see changelog

* Sun Jun 24 2007 Robos  <robos@muon.de>
- 1.0.2: -see changelog

* Mon Nov 13 2006 Robos  <robos@muon.de>
- 1.0.1: -see changelog

* Sun Apr 2 2006 Robos  <robos@muon.de>
- 1.0.0: -see changelog

* Wed Dez 7 2005 Robos  <robos@muon.de>
- 0.5.16: -see changelog

* Fri Jul 29 2005 Robos  <robos@muon.de>
- 0.5.15: -option to skip already present files with -m. 
  	  copying of dvd's with files ending in ";?" should work now.

* Sun Oct 24 2004 Robos  <robos@muon.de>
- 0.5.14-rc1: - misc *bsd fixes and first straight OSX support

* Mon Mar 7 2004 Robos  <robos@muon.de>
- 0.5.12-1: -m off-by-one error fixed

* Mon Jan 19 2004 Robos <robos@muon.de>
- 0.5.10-1: -O now works 
  	    cleanup



* Wed Nov 13 2003 Robos <robos@muon.de>
- 0.5.9-1: -F now accepts factor number
  	   cleanups and small bugfix
  	   new vobcopy.spec

* Sun Nov 09 2003 Florin Andrei <florin@andrei.myip.org>
- 0.5.8-2: libdvdread is now a pre-requisite

* Sun Nov 09 2003 Florin Andrei <florin@andrei.myip.org>
- first package, 0.5.8-1
