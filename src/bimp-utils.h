#ifndef __BIMP_UTILS_H__
#define __BIMP_UTILS_H__

#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include "bimp-manipulations.h"

char* bimp_str_replace(char*, char*, char*);
char* bimp_comp_get_filename(char*);
float bimp_min(float, float);
gboolean bimp_str_contains_cins(char*, char*);
int cstring_cmp(const void*, const void*);
void bimp_write_manipulation(manipulation, gpointer);
GimpParamDef bimp_get_param_info(gchar*, gint);

#endif
