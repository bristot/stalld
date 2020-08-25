NAME	:= starved
VERSION	:= 1.0


INSTALL=install
CFLAGS ?= -Wall -O2 -g

DIRS	:=	src redhat
FILES 	:=	Makefile README.md
TARBALL	:=	$(NAME)-$(VERSION).tar.xz

all: src/starved.o
	$(CC) -o starved -ggdb -lpthread src/starved.o

.PHONY: install
install:
	$(INSTALL) -m 755 -d $(DESTDIR)/usr/bin $(DESTDIR)/usr/share/$(NAME)-$(VERSION)
	$(INSTALL) starved -m 755 $(DESTDIR)/usr/bin/
	$(INSTALL) README.md -m 644 $(DESTDIR)/usr/share/$(NAME)-$(VERSION)

.PHONY: clean tarball redhat
clean:
	@test ! -f starved || rm starved
	@test ! -f src/starved.o || rm src/starved.o
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
