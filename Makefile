NAME	:=	stalld
VERSION	:=	1.0

INSTALL	=	install
CC	:=	gcc
CFLAGS	:=	-Wall -O2 -g
LDFLAGS	:=	-ggdb
LIBS	:=	 -lpthread

DIRS	:=	src redhat man
FILES	:=	Makefile README.md gpl-2.0.txt
TARBALL	:=	$(NAME)-$(VERSION).tar.xz
UPSTREAM_TARBALLS	:= fedorapeople.org:~/public_html/
BINDIR	:=	/usr/bin
DATADIR	:=	/usr/share
DOCDIR	:=	$(DATADIR)/doc
MANDIR	:=	$(DATADIR)/man
LICDIR	:=	$(DATADIR)/licenses

all: src/stalld.o
	$(CC) -o stalld	 $(LDFLAGS) src/stalld.o $(LIBS)

static: src/stalld.o
	$(CC) -o stalld-static $(LDFLAGS) --static src/stalld.o $(LIBS)

.PHONY: install
install:
	$(INSTALL) -m 755 -d $(DESTDIR)$(BINDIR) $(DESTDIR)$(DOCDIR)
	$(INSTALL) stalld -m 755 $(DESTDIR)$(BINDIR)
	$(INSTALL) README.md -m 644 $(DESTDIR)$(DOCDIR)
	$(INSTALL) -m 755 -d $(DESTDIR)$(MANDIR)/man8
	$(INSTALL) man/stalld.8 -m 644 $(DESTDIR)$(MANDIR)/man8
	$(INSTALL) -m 755 -d $(DESTDIR)$(LICDIR)/$(NAME)
	$(INSTALL) gpl-2.0.txt -m 644 $(DESTDIR)$(LICDIR)/$(NAME)

.PHONY: clean tarball redhat push
clean:
	@test ! -f stalld || rm stalld
	@test ! -f stalld-static || rm stalld-static
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
