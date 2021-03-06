Name:		stalld
Version:	1.2
Release:	1%{?dist}
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
* Mon Nov  2 2020 Clark Williams <williams@redhat.com> - 1.2-1
- utils.c: added info() functions
- detect and correctly parse old-style /proc/sched_debug
- src/stalld: Fix an retval check while reading sched_debug
- src/throttling: Fix a compilation warning
- ensure we only count task lines in old-format sched_debug info
- Add comments, clean up trailing whitespace
- src/utils: Fix runtime parameters check
- stalld: Do not take actions if log_only is set
- remove warning from parse_old_task_format

* Tue Oct 27 2020 Clark Williams <williams@redhat.com> - 1.1-1
- Fix an option in README.md; consistency in user facing docs.
- Makefile: add 'static' target to link stalld statically
- gitignore: ignore object files and the stalld executable
- use FIFO for boosting (v3)
- stalld.c: fix sched_debug parsing and modify waiting task parsing
- redhat:  update release for features and bugfix
- stalld: Do not die if sched_debug returns an invalid value
- src/stalld: Do not die if the comm is too large
- src/stalld: Do not die if cannot write a message to the log
- src/stalld: Do not die if the main runs while a thread is monitoring the CPU
- implement RT throttling management and refactor source files
- more refactoring
- src/stalld: Reuse already read nr_running nr_rt_running
- src/stalld: Gracefully handle CPUs not found on sched_debug
- src/stalld: Use dynamically allocated memory to read sched_debug
- src/utils: Die with a divizion by zero if verbose
- src/stalld: Add config_buffer_size variable
- src/stalld: Increase the sched_debug read buffer if it gets too small
- src/stalld: Fix an retval check while reading sched_debug
- src/throttling: Fix a compilation warning

* Sun Oct  4 2020 Clark Williams <williams@redhat.com> - 1.0-4
- Fix an option in README.md; consistency in user facing docs.
- gitignore: ignore object files and the stalld executable
- Makefile: add 'static' target to link stalld statically
- use FIFO for boosting (v3)
- stalld: update usage message to include --force_fifo/-F option
- stalld.c: fix sched_debug parsing and modify waiting task parsing

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
