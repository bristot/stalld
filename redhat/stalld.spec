Name:		stalld
Version:	1.0
Release:	3%{?dist}
Summary:	Daemon that finds starving tasks and gives them a temporary boost

License:	GPLv2
URL:		https://git.kernel.org/pub/scm/utils/stalld/stalld.git
Source0:	https://jcwillia.fedorapeople.org/%{name}-%{version}.tar.xz

BuildRequires:	glibc-devel
BuildRequires:	gcc
BuildRequires:	make
BuildRequires:	systemd-rpm-macros

Requires:	systemd

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
%make_build CFLAGS="%{build_cflags}" LDFLAGS="%{build_ldflags}"

%install
%make_install DOCDIR=%{_docdir} MANDIR=%{_mandir} BINDIR=%{_bindir} DATADIR=%{_datadir}
%make_install -C redhat UNITDIR=%{_unitdir}

%files
%{_bindir}/%{name}
%{_unitdir}/%{name}.service
%config(noreplace) %{_sysconfdir}/sysconfig/stalld
%doc %{_docdir}/README.md
%doc %{_mandir}/man8/stalld.8*
%license gpl-2.0.txt

%post
%systemd_post %{name}.service

%preun
%systemd_preun %{name}.service

%postun
%systemd_postun_with_restart %{name}.service

%changelog
* Tue Sep  1 2020 Clark Williams <williams@redhat.com> - 1.0-3
- Place BuildRequires on individual lines
- Fix changelog notations
- Modify build command to pass in CFLAGS and LDFLAGS
- fix compiler warnings in stalld.c

* Mon Aug 31 2020 Clark Williams <williams@redhat.com> - 1.0-2
- use _docdir macro for README.md
- use _mandir macro for stalld.8 manpage
- use tabs for spacing
- added push Makefile target to copy latest to upstream URL

* Tue Aug 25 2020 Clark Williams <williams@redhat.com> - 1.0-1
- rename project to stalld
- set version to 1.0
- clean up rpmlint complaints

* Fri Aug 21 2020 Clark Williams <williams@redhat.com> - 0.2-1
- add pidfile logic

* Thu Aug 20 2020 Clark Williams <williams@redhat.com> - 0.1-1
- Added systemd service to redhat subdirectory
- added make and rpm logic for systemd files

* Wed Aug 19 2020 Clark Williams <williams@redhat.com> - 0.0-1
- initial version of specfile
- Makefile mods for RPM builds
- added systemd service and config files
