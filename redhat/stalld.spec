Name:		stalld
Version:	%(grep ^VERSION ../Makefile | awk '{print $3}')
Release:	1%{?dist}
Summary:	Daemon that finds starving tasks and gives them a temporary boost

License:	GPLv2
URL:		https://github.com/bristot/stalld
Source0:	%{name}-%{version}.tar.xz

BuildRequires: glibc-devel
Requires:      systemd

%description
The stalld program monitors the set of system threads,
looking for threads that are ready-to-run but have not
been given processor time for some threshold period.
When a starving thread is found, it is given a temporary
boost using the SCHED_DEADLINE policy. The default is to
allow 10 microseconds of runtime for 1 second of clock time.

%prep
%autosetup


%build
%make_build


%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install
make DESTDIR=$RPM_BUILD_ROOT -C redhat install

%files
%{_bindir}/%{name}
%{_unitdir}/%{name}.service
%config(noreplace) /etc/systemd/stalld.conf
%doc %{_datadir}/%{name}-%{version}/README.md
%doc %{_datadir}/man/man8/stalld.8.gz

%changelog
* Tue Aug 25 2020 williams@redhat,com - 1.0-1
- rename project to stalld
- set version to 1.0
- clean up rpmlint complaints

* Fri Aug 21 2020 williams@redhat.com - 0.2-1
- add pidfile logic

* Thu Aug 20 2020 williams@redhat.com - 0.1-1
- Added systemd service to redhat subdirectory
- added make and rpm logic for systemd files

* Wed Aug 19 2020 williams@redhat.com - 0.0-1
- initial version of specfile
- Makefile mods for RPM builds
- added systemd service and config files
