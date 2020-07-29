INSTALL=install
CFLAGS ?= -Wall

all: src/starvation_monitor.o
	$(CC) -o starvation_monitor -ggdb -lpthread src/starvation_monitor.o

.PHONY: install
install:
	$(INSTALL) starvation_monitor -m 755 $(DESTDIR)/usr/bin/

.PHONY: clean
clean:
	@test ! -f starvation_monitor || rm starvation_monitor
	@test ! -f src/starvation_monitor.o || rm src/starvation_monitor.o
