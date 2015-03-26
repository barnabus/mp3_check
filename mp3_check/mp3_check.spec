Summary: A utility which analyzes mp3 files for errors and standards conformance. 
Name: mp3_check 
Version: 1.95 
Release: 1 
Group: Applications/File 
Copyright: GPL
Source: ftp://ftp.thedeepsky.com/outgoing/mp3_check-2.0.tar.gz
Buildroot: /var/tmp/mp3_check-root

%description
Identify in explicit detail mp3s that do not correctly
follow the mp3 format. Also look for invalid frame
headers, missing frames, etc. This can be especially
important when building an archive, and you want 
quality mp3s.
								
%prep
%setup

%build
make

%install
mkdir -p $RPM_BUILD_ROOT/usr/local/bin
install -s -m 755 -o root -g root mp3_check $RPM_BUILD_ROOT/usr/local/bin

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/local/bin/mp3_check
