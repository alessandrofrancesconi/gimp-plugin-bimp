#ifndef __BIMP_H__
#define __BIMP_H__

#include <gtk/gtk.h>

#define PLUG_IN_FULLNAME "Batch Image Manipulation Plugin"
#define PLUG_IN_DESCRIPTION "Applies GIMP manipulations on group of images"
#define PLUG_IN_BINARY "bimp"
#define PLUG_IN_PROC "plug-in-bimp"
#define PLUG_IN_VERSION_MAJ 0
#define PLUG_IN_VERSION_MIN 4

GSList *bimp_input_filenames;
char *bimp_output_folder;
gboolean bimp_alertoverwrite;

#endif
