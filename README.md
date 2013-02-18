BIMP. Batch Image Manipulation Plugin for GIMP.
===============================================

With BIMP you can apply a set of GIMP manipulations on groups of images.
Documentation @ http://www.alessandrofrancesconi.it/projects/bimp


Compiling and installing under Linux/Unix
-----------------------------------------

You must install libgimp2.0-dev and libpcre3-dev packages in order to have 
the full set of libraries and dependences for compiling BIMP.
Then:

	make && make install
	
Or:

	make && sudo make install-admin

To make and install for every user in the system (needs root privileges).

1.	Make sure that the "bimp-locale" folder is copied in the plug-in directory too! If not, you can do it manually by copying it from ./bin/win32.
2.	Don't mind the warnings...


Installing under Windows
-------------------------

BIMP plugin for Windows is already compiled and it's saved in bin/win32 directory.
Just copy bimp.exe, pcre3.dll and bimp-locale into the default GIMP-plugins folder:
`<Programs-dir>\<GIMP-folder>\lib\gimp\<version>\plug-ins`
or
`<User-dir>\.gimp<version>\plug-ins`


Support this project
--------------------

Visit http://github.com/alessandrofrancesconi/gimp-plugin-bimp/issues
for posting bugs and enhancements. Make it better!
