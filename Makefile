NAME	:= stalld
VERSION	:= 1.0


INSTALL=install
CFLAGS ?= -Wall -O2 -g

DIRS	:=	src redhat man
FILES 	:=	Makefile README.md
TARBALL	:=	$(NAME)-$(VERSION).tar.xz
UPSTREAM_TARBALLS	:= fedorapeople.org:~/public_html/

all: src/stalld.o
	$(CC) -o stalld -ggdb -lpthread src/stalld.o

.PHONY: install
install:
	$(INSTALL) -m 755 -d $(DESTDIR)/usr/bin $(DESTDIR)/usr/share/$(NAME)-$(VERSION)
	$(INSTALL) stalld -m 755 $(DESTDIR)/usr/bin/
	$(INSTALL) README.md -m 644 $(DESTDIR)/usr/share/$(NAME)-$(VERSION)
	$(INSTALL) -m 755 -d $(DESTDIR)/usr/share/man/man8
	$(INSTALL) man/stalld.8 -m 644 $(DESTDIR)/usr/share/man/man8

.PHONY: clean tarball redhat push
clean:
	@test ! -f stalld || rm stalld
	@test ! -f src/stalld.o || rm src/stalld.o
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

push: tarball
	scp $(TARBALL) $(UPSTREAM_TARBALLS)
	make -C redhat push
