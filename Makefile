VERSION=0.1

all: cryptsetup-gui cryptsetup-gui-gtk

archive:
	mkdir "src"
	cp cryptsetup-gui.c cryptsetup-gui-gtk.c xinitrc Makefile src/
	tar czvf cryptsetup-gui-$(VERSION).tar.gz -C src .
	rm -rf src

install: all
	mkdir -p "$(DESTDIR)/usr/local/bin"
	cp cryptsetup-gui cryptsetup-gui-gtk "$(DESTDIR)/usr/local/bin"
	cp xinitrc "$(DESTDIR)/etc/skel/.xinitrc-cryptsetup-gui"

cryptsetup-gui: cryptsetup-gui.c
	gcc -D_GNU_SOURCE -Wall -O2 -g cryptsetup-gui.c -o cryptsetup-gui

cryptsetup-gui-gtk: cryptsetup-gui-gtk.c
	gcc -Wall -O2 -g -D_GNU_SOURCE cryptsetup-gui-gtk.c -o cryptsetup-gui-gtk `pkg-config --cflags --libs gtk+-2.0`
