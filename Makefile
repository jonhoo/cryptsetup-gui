VERSION=0.2

all: cryptsetup-gui cryptsetup-gui-gtk

archive:
	mkdir "cryptsetup-gui-$(VERSION)"
	cp cryptsetup-gui.c cryptsetup-gui-gtk.c xinitrc Makefile "cryptsetup-gui-$(VERSION)"
	tar czvf cryptsetup-gui-$(VERSION).tar.gz "cryptsetup-gui-$(VERSION)"
	rm -rf "cryptsetup-gui-$(VERSION)"

install: all
	mkdir -p "$(DESTDIR)/usr/bin"
	cp cryptsetup-gui cryptsetup-gui-gtk "$(DESTDIR)/usr/bin"
	chmod 0755 $(DESTDIR)/usr/bin/cryptsetup-gui-gtk
	chmod 0750 $(DESTDIR)/usr/bin/cryptsetup-gui
	chmod u+s $(DESTDIR)/usr/bin/cryptsetup-gui
	chown root:users $(DESTDIR)/usr/bin/cryptsetup-gui

	mkdir -p "$(DESTDIR)/etc/skel/"
	cp xinitrc "$(DESTDIR)/etc/skel/.xinitrc-cryptsetup-gui"

cryptsetup-gui: cryptsetup-gui.c
	gcc -D_GNU_SOURCE -Wall -O2 -g cryptsetup-gui.c -o cryptsetup-gui

cryptsetup-gui-gtk: cryptsetup-gui-gtk.c
	gcc -Wall -O2 -g -D_GNU_SOURCE cryptsetup-gui-gtk.c -o cryptsetup-gui-gtk `pkg-config --cflags --libs gtk+-2.0`
