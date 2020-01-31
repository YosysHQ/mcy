DESTDIR =
PREFIX = /usr/local

build:
	cd gui && cmake -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	$(MAKE) -C gui

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install mcy.py $(DESTDIR)$(PREFIX)/bin/mcy
	install mcy-dash.py $(DESTDIR)$(PREFIX)/bin/mcy-dash
	mkdir -p $(DESTDIR)$(PREFIX)/share/mcy/dash
	cp -r dash/. $(DESTDIR)$(PREFIX)/share/mcy/dash/.	
	$(MAKE) -C gui install

html:
	make -C docs html
