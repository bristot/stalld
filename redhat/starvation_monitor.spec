Name:		starvation_monitor
Version:	0.1
Release:	1%{?dist}
Summary:	daemon that finds starving tasks and gives them a temporary boost

License:	GPL
URL:		https://github.com/bristot/starvation_monitor
Source0:	%{name}-%{version}.tar.xz

BuildRequires: glibc-devel
Requires:      systemd

%description
The starvation_monitor program monitors the set of system threads, looking for threads
that are ready-to-run but have not been given cpu time for some threshold period. When
a starving thread is found, it is given a temporary boost using the SCHED_DEADLINE
policy. The default is to allow 10 microseconds of runtime for 1 second of clock time.

%prep
%autosetup


%build
%make_build


%install
rm -rf $RPM_BUILD_ROOT
%make_install


%files
/usr/bin/%{name}
%doc /usr/share/%{name}-%{version}/README.md


%changelog
* Wed Aug 19 2020 williams@redhat.com
-
