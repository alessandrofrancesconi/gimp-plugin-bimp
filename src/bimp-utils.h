#ifndef __BIMP_UTILS_H__
#define __BIMP_UTILS_H__

#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include "bimp-manipulations.h"

char* str_replace(char*, char*, char*);
char* comp_get_filename(char*);
char* comp_get_filefolder(char*);
gboolean str_contains_cins(char*, char*);
GimpParamDef pdb_proc_get_param_info(gchar*, gint);
char* get_user_dir(void); 
char* get_bimp_localedir(void);
int glib_strcmpi(gconstpointer, gconstpointer);

#if defined _WIN32
#define FILE_SEPARATOR '\\'
#define FILE_SEPARATOR_STR "\\"
#else
#define FILE_SEPARATOR '/'
#define FILE_SEPARATOR_STR "/"
#endif

#define min(a,b) (a < b ? a : b)

#endif
