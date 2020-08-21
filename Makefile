NAME	:= starvation_monitor
VERSION	:= 0.1


INSTALL=install
CFLAGS ?= -Wall -O2 -g

DIRS	:=	src redhat
FILES 	:=	Makefile README.md
TARBALL	:=	$(NAME)-$(VERSION).tar.xz

all: src/starvation_monitor.o
	$(CC) -o starvation_monitor -ggdb -lpthread src/starvation_monitor.o

.PHONY: install
install:
	$(INSTALL) -m 755 -d $(DESTDIR)/usr/bin $(DESTDIR)/usr/share/$(NAME)-$(VERSION)
	$(INSTALL) starvation_monitor -m 755 $(DESTDIR)/usr/bin/
	$(INSTALL) README.md -m 644 $(DESTDIR)/usr/share/$(NAME)-$(VERSION)

.PHONY: clean tarball redhat
clean:
	@test ! -f starvation_monitor || rm starvation_monitor
	@test ! -f src/starvation_monitor.o || rm src/starvation_monitor.o
	@test ! -f $(TARBALL) || rm -f $(TARBALL)
	@make -C redhat clean
	@rm -rf *~

tarball:  clean
	rm -rf $(NAME)-$(VERSION) && mkdir $(NAME)-$(VERSION)
	cp -r $(DIRS) $(FILES) $(NAME)-$(VERSION)
	tar -cvJf $(TARBALL) --exclude='*~' $(NAME)-$(VERSION)
	rm -rf $(NAME)-$(VERSION)

redhat: tarball
	$(MAKE) -C redhat
