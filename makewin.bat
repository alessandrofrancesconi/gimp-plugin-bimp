@echo off

set libdir=C:\dev
set gtk=gtk-dev
set gimp=gimp-dev
set pcre=pcre-dev

set mingwdir=C:\dev\mingw
set gccdir=%mingwdir%\bin
set msysdir=%mingwdir%\msys\1.0\bin
set compPATH=%gccdir%;%msysdir%;%libdir%\%gimp%\bin;%libdir%\%gtk%\bin;%libdir%\%pcre%\bin;

set PATH=%PATH%;%compPATH%
setlocal EnableDelayedExpansion

set SOURCES=
for /r %%c in (src\*.c) DO call set "SOURCES=%%SOURCES%% %%c"
for /r %%c in (src\manipulation-gui\*.c) DO call set "SOURCES=%%SOURCES%% %%c"

@echo on

mkdir bin\win32
%gccdir%\gcc.exe -o bin\win32\bimp.exe -Wall -O2 -Wno-unused-variable -Wno-pointer-sign -Wno-parentheses %SOURCES% -L%libdir%\%gtk%\lib -lgtk-win32-2.0 -lgdk-win32-2.0 -latk-1.0 -lgio-2.0 -lpangowin32-1.0 -lgdi32 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lpango-1.0 -lcairo -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0 -lintl -L%libdir%\%gimp%\lib -L%libdir%\%pcre%\lib -lgimpui-2.0 -lgimpwidgets-2.0 -lgimpmodule-2.0 -lgimp-2.0 -lgimpmath-2.0 -lgimpconfig-2.0 -lgimpcolor-2.0 -lgimpbase-2.0 -lpcre -mms-bitfields -mwindows -m32 -I%libdir%\%gtk%\include\gtk-2.0 -I%libdir%\%gtk%\lib\gtk-2.0\include -I%libdir%\%gtk%\include\atk-1.0 -I%libdir%\%gtk%\include\cairo -I%libdir%\%gtk%\include\gdk-pixbuf-2.0 -I%libdir%\%gtk%\include\pango-1.0 -I%libdir%\%gtk%\include\glib-2.0 -I%libdir%\%gtk%\lib\glib-2.0\include -I%libdir%\%gtk%\include -I%libdir%\%gtk%\include\freetype2 -I%libdir%\%gtk%\include\libpng14 -I%libdir%\%gimp%\include\gimp-2.0 -I%libdir%\%pcre%\include -DGIMP_DISABLE_DEPRECATED
