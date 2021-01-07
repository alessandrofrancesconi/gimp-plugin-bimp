BIMP. Batch Image Manipulation Plugin for GIMP.
===============================================

Changelog
---------

2.5
 - Fixed Color Curve not working if exported using GIMP 2.10 (issue #225)
 - Fixed Windows uninstaller not working in both normal and silent mode (issues #258 #240 #196)
 
2.4
 - Added support to OpenEXR images
 
2.3
 - Added Dutch translation (thanks to Just Vecht & Paul Matthijsse from Dutch Helpmij Magazine)
 - Fixed zh-CN locale (thanks to @KrasnayaPloshchad)
 
2.2
 - Added support to WEBP (issue #184)
 - Enabled HEIF in "Change Format and Compression" (issue #185)
 - Saving back to HEIF with default values caused "zero quality" image
 - Fixed TIFF parameter setup in "Change Format and Compression"
 - Removed PCRE dependency in favor of GLib (issue #189)
 
2.1
 - Improved GUI flexibility and high DPI support (thanks to @scribblemaniac)
 - Added Serbian language (thanks to @nstVanja)
 - Fixed spurious tick char generation in makefile (thanks to @ciampix)
 - Fixed black images after Color Curve

2.0 (aka "hey what's up?")
 - BIMP codebase has been updated to be natively compatible with GIMP 2.10, while it does not work anymore with GIMP <= 2.8.
 - Added support to HEIF, XPM and JPEG2000 images (#182, #141, #138)
 - Use GResources instead of Pixdata
 - Many compiler warnings fixed
 - Enhanced Win32 make procedure (needs MSYS2)

1.18
 - Windows installer updated to better handle errors (#131)
 - ICO and TGA added as image input (#126, #127) 
 - Added Norwegian translation (thanks to Kolbjørn Stuestøl)
 - Updated Spanish (thanks to Damián Blanco) and Russian (thanks to @Skif-off) languages

1.17
 - Fixed an error when parsing "Other GIMP procedure" parameters that contain only a "=" sign (like Despeckle plugin) (thanks Daniel B Shak)
 - Disable "undo" during processing for better performances (issue #109)
 - Fixed error when using "Fill with Padding" on an indexed image (issue #98)
 - Korean translation (thanks to Jun Park) and Chinese Traditional translation (thanks to @andy0130tw)

1.16 (aka "I'm expecting 100.000 donations for this")
 - Changed layout and added "Fill with padding" option in Resize manipulation (issue #62, #85)
 - Changed layout and added "Adaptive size" (issue #90) and "Distance to edge" options in Watermark manipulation
 - Remember last selected folder in file chooser (issue #96)
 - Small performance improvements when using Resize
 - Removed title from manipulation windows (redundant with system's GUI)
 - Updated Win32 installer with optimization for silent mode and other checks
 - Added Catalan translation (thanks to Xavier Beà) and Chinese translation (thanks to kissbug8720)

1.15
 - BIMP finally has an official installer for Windows!
 - Faster startup
 - Added Japanese translation

1.14
 - Added start position chooser for Crop (issue #81, thanks to @thief425)
 - Added support to ITEM datatype in "Other GIMP procedure" (issue #74)
 - JPEG conversion always set the image to RGB (issue #88)
 - Alpha channel is no more added by default during process
 - Fixed error when importing an opened GIMP image that was not yet saved on file system
 - Updated German translation (thanks to Christian Kühnemund)
 
1.13
 - Fixed calling errors for procedure 'gimp-image-convert-indexed'
 - Color correction not correctly readed from file
 - Spanish translation (thanks to Damián Blanco)
 - Updated French translation

1.12
 - New layout for add/remove files buttons
 - "Add all opened images" feature (#57)
 - "Keep modification dates" option (#70)
 - GIF images was not saved when using "Color Correction"
 - Updated Italian, Polish and Czech translations
 - Little code and memory optimizations

1.11 ONEONE!!ONE!ONE!!ONE!!
 - Added Czech translation (thanks to Stanislav Vágner)
 - JPEG DCT settings was wrong (issue #64)
 - Resize interpolation was not saved during "Save set" process (issue #63)

1.10
 - Changed the behaviour of the new "current source folder as output destination" button. Now it takes 
   the location of the current selected file as the output folder.

1.9
 - Added watermark alignment buttons for top-center, center-left, center-right, bottom-center (thanks to @Walt9Z)
 - It's now possible to resize both dimension but keeping the aspect ratio. In this case, the pixel width/height 
   are treated as maximum sizes for the resulting image. (issue #25) (thanks to @Walt9Z)
 - Resize and Crop manipulations are now treated as "normal priority" steps. Previously they started first, now they follow
   the original order given by the user in the GUI (issue #53)
 - It's now possible to select the current source folder as the output destination (only if every input file is located in it).
 - Added "tall" smartphone screen Crop preset (for devices like iPhone 5+)
 - JPEG subsampling settings was wrong (issue #52)
 - Updated Italian translation
   
1.8
 - Unrecognized capital file extensions on Unix

1.7
 - Replaced buggy GtkFileChooserButton with GtkButton to select output folder (should be the end of the nightmare) (issue #42 and #43)
 - Changed overwrite confirmation and moved image modifications after overwriting check (thanks to Thomas Mevel @mevtho) (issue #50)
 - Updated Italian translation
 
1.6
 - Curve parser can't load some kinds of GIMP Curve file (issue #46)

1.5:
 - Added external Curve file chooser in Color manipulation (issue #46)
 - Preview GUI didn't display Resize, Crop and Watermark manipulation results
 - Manipulation windows have now better size (should fix issue #24)

1.4:
 - Fixed a bug that displayed (none) in output folder button and caused BIMP to crash
 - Fixed a compilation error under Unix
 - Some little optimizations in batch process

1.3:
 - Changed the way the FileChooser lets you pick image files and folders (should solve issues #23 and #35)
 - Enabled multiple selection in the input files panel
 - NOTE for Mac OSX users: There's no need to install or compile BIMP on Mac anymore, see the README

1.2:
 - BIMP can now accept SVG images as input
 - Updated French, Italian and Portuguese translations

1.1:
 - Possibility to retrieve images from the whole hierarchy when opening a folder (thanks to Thomas Mevel @mevtho)
 - Added a folder hierarchy saving in the output folder (thanks to Thomas Mevel @mevtho)
 - (temporary) fix for fail-start in GIMP 2.6 for Windows
 - Fixed wrong limits interpretation in "Other GIMP procedure" window (issue #31)
 - Fixed crash when converting image files without extension into another format (partially fixes #28)
 - Fixed crash when adding input folders that contain files with special chars
 - Updated French and Italian translations
 - Better Makefile for Linux (thanks to @ivoanjo)
 - Some code optimizations

1.0:
 - (finally) Added "Save set" and "Load set" features (issue #10)
 - Possibility to set a custom aspect ratio under "Crop"
 - Better floating point limits for GtkSpinButtons in "Other GIMP procedure..." (issue #27)
 - Updated Italian translation
 - Little code optimizations

0.9:
 - Portuguese and German translations (thanks to Pedro Cunha and Vito Zaccaria)
 - Updated French translation (thanks to Robert Brizard)
 - Fixed regression that caused BIMP to fail start on GIMP 2.6

0.8:
 - Multilanguage support (available languages: English, Italian, French) (issue #6)
 - Added Manipulation Preview window (issue #16)
 - Limits for GTK Spin Buttons in "Other GIMP procedure..." are now well interpreted (this also fixes issue #22)
 - Workaround for a bug that affects MacOS X users when selecting images (issue #23)
 - Support XCF as input format (issue #19)
 - Few graphical fixes around

0.7:
 - Added "Add folders" button for adding entire folders content in the input list

0.6:
 - Added STOP button to abort the batch process
 - List of options and modes in "Other GIMP procedure..." are now represented by a GtkComboBox
 - Code and speed optimizations on "Other GIMP procedure..." dialog
 - Added "Yes, don't ask again" option in "Overwrite?" dialog
 - Various little fixes around
 - BIMP now depends on the presence of pcre library (http://www.pcre.org/)
	-- Linux/Mac users: just install libpcre3-dev (or equivalent)
	-- Windows users: make sure you copy pcre3.dll library in the same directory of bimp.exe!

0.5:
 - Rewritten the "Other GIMP procedure..." dialog and made some stability fixes
 - More format customizations in "Change Format and Quality"
 - Fixed a bug that didn't permit to save images in TGA format
 - Few graphical fixes around

0.4:
 - Fixed a regression that causes RENAME procedure to crash.

0.3:
 - Added "Change Resolution" in Resize manipulation
 - Replaced "Sharpen" with "Sharp or blur"
 - More customizations in the "Change Format" dialog
 - In "Other GIMP procedure" dialog, 'toggle' values are now represented by a GtkToggleButton
 - Code and memory allocation optimizations

0.2:
 - Fixed "Other GIMP procedure" dialog crashes under Win32
 - Improved "Other GIMP procedure" dialog funcionalities (support to Font and Opacity params)
 - Nicer "Add Watermark" gui
 - Added files preview box
 - Added some documentation comments in source code
 - Few graphical fixes around

0.1:
 - It all started from here...
