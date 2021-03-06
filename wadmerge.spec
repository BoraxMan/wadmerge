Summary: Merges WAD file for Doom engine games.
Name: wadmerge
Version: 1.0.2
Release: 1
License: GPL
Group: System
Source: http://dennisk.customer.netspace.net.au/wadmerge/wadmerge-1.0.2.tar.gz
URL: http://dennisk.customer.netspace.net.au/wadmerge.html
Distribution: Fedora
Vendor: DK Soft
Packager: Dennis Katsonis <dennisk@netspace.net.au>
%global debug_package %{nil}

%description
Merges .WAD files from Doom engine games, such as
Doom, Doom2, Hexen and Heretic.

%prep
%setup

%build
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_docdir}/*
%{_bindir}/*
%{_mandir}/man1/*


%clean
rm -rf $RPM_BUILD_ROOT

