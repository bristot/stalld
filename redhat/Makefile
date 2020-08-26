# Red Hat specific Makefile

HERE	:= $(shell pwd)
RPMDIRS	:= SPECS SOURCES BUILDROOT RPMS SRPMS BUILD
RPMARGS	:= 	--define "_topdir	$(HERE)" \
		--define "_sourcedir	$(HERE)/SOURCES" \
		--define "_builddir 	$(HERE)/BUILD" \
		--define "_rpmdir	$(HERE)/RPMS" \
		--define "_srcrpmdir	$(HERE)/SRPMS"
INSTALL	:= install

all: rpm

rpmdirs:
	@[ -d SPECS ]	|| mkdir SPECS
	@[ -d SOURCES ]	|| mkdir SOURCES
	@[ -d BUILD ]	|| mkdir BUILD
	@[ -d RPMS ]	|| mkdir RPMS
	@[ -d SRPMS ]	|| mkdir SRPMS

rpm: rpmdirs
	cp ../*.tar.xz SOURCES
	rpmbuild -ba $(RPMARGS) stalld.spec

clean:
	@rm -rf $(RPMDIRS) *~

install:
	$(INSTALL) -m 755 -d $(DESTDIR)/etc/systemd
	$(INSTALL) stalld.conf -m 644 $(DESTDIR)/etc/systemd
	$(INSTALL) -m 755 -d $(DESTDIR)/usr/lib/systemd/system
	$(INSTALL) stalld.service -m 644 $(DESTDIR)/usr/lib/systemd/system

rpmlint:  stalld.spec
	rpmlint -i stalld.spec
