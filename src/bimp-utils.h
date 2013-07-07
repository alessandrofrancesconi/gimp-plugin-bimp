#ifndef __BIMP_UTILS_H__
#define __BIMP_UTILS_H__

#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include "bimp-manipulations.h"

char* bimp_str_replace(char*, char*, char*);
char* bimp_comp_get_filename(char*);
char* bimp_comp_get_filefolder(char*);
float bimp_min(float, float);
gboolean bimp_str_contains_cins(char*, char*);
void bimp_write_manipulation(manipulation, gpointer);
GimpParamDef bimp_get_param_info(gchar*, gint);

#if defined _WIN32
#define FILE_SEPARATOR '\\'
#define FILE_SEPARATOR_STR "\\"
#else
#define FILE_SEPARATOR '/'
#define FILE_SEPARATOR_STR "/"
#endif


#endif
