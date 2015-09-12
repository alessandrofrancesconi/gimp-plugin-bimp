BIMP. Batch Image Manipulation Plugin for GIMP.
===============================================

With BIMP you can apply a set of GIMP manipulations on groups of images.
Documentation @ http://www.alessandrofrancesconi.it/projects/bimp

![A screenshot of BIMP](http://www.alessandrofrancesconi.it/projects/bimp/images/bimp-main.jpg)

Installing on Windows
---------------------

BIMP can be easily installed on your Windows 32/64bit machine with the 
official installer: https://github.com/alessandrofrancesconi/gimp-plugin-bimp/releases/latest/
then download `gimp-plugin-bimp_win32.exe`

Instead... to remove BIMP... if you really want... just go to GIMP's plugins directory and remove 
`bimp.exe` and `bimp-locale` folder.


Compiling and installing on Linux
-----------------------------------------

You must install `libgimp2.0-dev` and `libpcre3-dev` packages in order to have 
the full set of libraries and dependences to compile BIMP (names can differ depending on the distro).

For Ubuntu / Linux Mint:

	sudo apt-get install libgimp2.0-dev libpcre3-dev

Extract the archive containing the BIMP's sources and get into the extracted folder with `cd`. Then:

	make && make install
	
Or:

	make && sudo make install-admin

to make and install for every user in the system (needs root privileges).


For Mac OSX users
-----------------
There's no need to install or compile BIMP on Mac, because the default native build of GIMP for Mac OSX 10.8 
made by Simone from http://gimp.lisanet.de/Website/Download.html already includes BIMP!
(however, it's not guaranteed to be the very latest version)


Support this project
--------------------

Visit http://github.com/alessandrofrancesconi/gimp-plugin-bimp/issues
and post alerts for bugs or enhancements. Make it better!
