
Debian
====================
This directory contains files used to package pqucoind/pqucoin-qt
for Debian-based Linux systems. If you compile pqucoind/pqucoin-qt yourself, there are some useful files here.

## pqucoin: URI support ##


pqucoin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install pqucoin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your pqucoin-qt binary to `/usr/bin`
and the `../../share/pixmaps/pqucoin128.png` to `/usr/share/pixmaps`

pqucoin-qt.protocol (KDE)

