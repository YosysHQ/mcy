DESTDIR =
PREFIX = /usr/local

build:
	cd gui && cmake -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	$(MAKE) -C gui
	cd gui-create && cmake -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	$(MAKE) -C gui-create

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/bin/generators
	install mcy.py $(DESTDIR)$(PREFIX)/bin/mcy
	install mcy-dash.py $(DESTDIR)$(PREFIX)/bin/mcy-dash
	install mcy-gen.py $(DESTDIR)$(PREFIX)/bin/mcy-gen
	cp -r generators/. $(DESTDIR)$(PREFIX)/bin/generators/.	
	mkdir -p $(DESTDIR)$(PREFIX)/share/mcy/dash
	cp -r dash/. $(DESTDIR)$(PREFIX)/share/mcy/dash/.	
	$(MAKE) -C gui install
	$(MAKE) -C gui-create install
