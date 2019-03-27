DESTDIR =
PREFIX = /usr/local

build:
	cd gui && cmake -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	$(MAKE) -C gui

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install mcy.py $(DESTDIR)$(PREFIX)/bin/mcy
	$(MAKE) -C gui install
