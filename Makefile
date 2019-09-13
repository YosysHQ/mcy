DESTDIR =
PREFIX = /usr/local

build:
	cd gui && cmake -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	$(MAKE) -C gui
	cd gui-create && cmake -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	$(MAKE) -C gui-create

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install mcy.py $(DESTDIR)$(PREFIX)/bin/mcy
	install mcy-dash.py $(DESTDIR)$(PREFIX)/bin/mcy-dash
	mkdir -p $(DESTDIR)$(PREFIX)/share/mcy/dash
	cp -r dash/. $(DESTDIR)$(PREFIX)/share/mcy/dash/.	
	$(MAKE) -C gui install
	$(MAKE) -C gui-create install
