#ifndef __BIMP_UTILS_H__
#define __BIMP_UTILS_H__

#include <gtk/gtk.h>

char* bimp_str_replace(char*, char*, char*);
char* bimp_comp_get_filename(char*);
char* bimp_get_datetime(void);
float bimp_min(float, float);
gboolean bimp_str_contains_cins(char*, char*);
int cstring_cmp(const void*, const void*);
#endif
