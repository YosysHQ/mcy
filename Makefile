DESTDIR =
PREFIX = /usr/local

help:
	@echo ""
	@echo "sudo make install"
	@echo "    build and install SymbiYosys (sby)"
	@echo ""

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install mcy.py $(DESTDIR)$(PREFIX)/bin/mcy
