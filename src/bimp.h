#ifndef __BIMP_H__
#define __BIMP_H__

#include <gtk/gtk.h>

#define PLUG_IN_FULLNAME "Batch Image Manipulation Plugin"
#define PLUG_IN_DESCRIPTION "Applies GIMP manipulations on groups of images"
#define PLUG_IN_COPYRIGHT "(C) 2016 - Alessandro Francesconi"
#define PLUG_IN_WEBSITE "http://www.alessandrofrancesconi.it/projects/bimp"
#define PLUG_IN_BINARY "bimp"
#define PLUG_IN_PROC "plug-in-bimp"
#define PLUG_IN_VERSION "1.18"

#define BIMP_RESULT_OK 1
#define BIMP_RESULT_WARNING 0
#define BIMP_RESULT_ERROR -1

#define BIMP_ASK_OVERWRITE 0
#define BIMP_OVERWRITE_SKIP_ASK 1
#define BIMP_DONT_OVERWRITE_SKIP_ASK 2

GSList *bimp_input_filenames;
char *bimp_output_folder;

gint bimp_opt_alertoverwrite;
gboolean bimp_opt_keepfolderhierarchy;
gboolean bimp_opt_deleteondone;
gboolean bimp_opt_keepdates;

gboolean bimp_is_busy;

GSList *bimp_supported_procedures;
void init_supported_procedures(void);

#endif

