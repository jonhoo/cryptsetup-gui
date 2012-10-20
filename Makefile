all: cryptsetup-gui cryptsetup-gui-gtk

cryptsetup-gui: cryptsetup-gui.c
	gcc -D_GNU_SOURCE -Wall -O2 -g cryptsetup-gui.c -o cryptsetup-gui

cryptsetup-gui-gtk: cryptsetup-gui-gtk.c
	gcc -Wall -O2 -g -D_GNU_SOURCE cryptsetup-gui-gtk.c -o cryptsetup-gui-gtk `pkg-config --cflags --libs gtk+-2.0`
