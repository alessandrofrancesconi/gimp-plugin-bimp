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

*Note for GIMP Portable users:* If you experience strange things like missing buttons or totally white windows, 
first ensure you have installed the latest version of the GTK+ runtime (download it from [here](https://sourceforge.net/projects/gtk-win/files/latest/download?source=files)).

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
(however, it's not guaranteed to be the very latest version of either BIMP or GIMP)

For those interested in remaining on the cutting edge, the instructions are as follows:

1. Install [MacPorts](https://www.macports.org/install.php)
2. Install prerequisites: `sudo port install coreutils pcre `
3. Add `/opt/local/libexec/gnubin` to your `PATH` to make them the GNU tools the default: `PATH=/opt/local/libexec/gnubin:$PATH`  (You can also add this to your login profile if you want the change to be permanent).
3. Install GIMP with MacPorts: `sudo port install gimp +quartz` (You can leave off the `+quartz` if you prefer GIMP to run in the X11 environment instead of natively.)
4. Follow the build and install instructions for Linux.
5. Look at the output for the install command.  If your plug-in directory contains a space in its name (as would happen if it is in the `Application Support` folder), then there will be a copy command which failed listed.  You will need to perform that copy command manually with the destination directory properly wrapped in quotes.  I.e. if you see
```
cp ./bin/bimp /Users/NAME/Library/Application Support/GIMP/2.8/plug-ins
cp: target 'Support/GIMP/2.8/plug-ins' is not a directory
```
then you need to do `cp ./bin/bimp "/Users/NAME/Library/Application Support/GIMP/2.8/plug-ins"`.

*Note:* Even though you have to install GIMP from MacPorts in order to build the binaries for BIMP, they should work just fine with the self-contained GIMP build from gimp.org.  In fact, you could probably uninstall the MacPorts version once the binaries are built, but you'll need to reinstall it each time you want to update BIMP.


Support this project
--------------------

Visit http://github.com/alessandrofrancesconi/gimp-plugin-bimp/issues
and post alerts for bugs or enhancements. Make it better!
