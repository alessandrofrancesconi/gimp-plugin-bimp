/* some utility functions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <libgimp/gimp.h>
#include "bimp-utils.h"

/* replace all the occurrences of 'rep' into 'orig' with text 'with' */
char* bimp_str_replace(char *orig, char *rep, char *with) 
{
    char *result;
    char *ins;
    char *tmp;
    int len_rep;
    int len_with;
    int len_front;
    int count;

    if (!orig) {
        return NULL;
	}
    if (!rep || !(len_rep = strlen(rep))) {
        return NULL;
	}
    if (!(ins = strstr(orig, rep))) {
        return NULL;
	}
	
    if (!with) {
        with = "";
    }
    
    len_with = strlen(with);

    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result) {
        return NULL;
	}
	
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep;
    }
    strcpy(tmp, orig);
    return result;
}

/* gets the filename from the given path 
 * (compatible with unix and win) */
char* bimp_comp_get_filename(char* path) 
{
	char *pfile;
	
	pfile = path + strlen(path);
    for (; pfile > path; pfile--)
    {
        if ((*pfile == '\\') || (*pfile == '/'))
        {
            pfile++;
            break;
        }
    }
    
    return pfile;
}

/* gets the current date and time in "%Y-%m-%d_%H-%M" format */
char* bimp_get_datetime() {
	time_t rawtime;
	struct tm * timeinfo;
	char* format;

	format = (char*)malloc(sizeof(char)*18);
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	strftime (format, 18, "%Y-%m-%d_%H-%M", timeinfo);

	return format;
}

/* simply the min function */
float bimp_min(float a, float b) {
	if (a < b)
		return a;
	else
		return b;
}

/* return TRUE if the first string 'fullstr' contains one or more occurences of substring 'search'.
 * (case-insensitive version) */
gboolean bimp_str_contains_cins(char* fullstr, char* search) {
	return (
		strstr(
			g_ascii_strdown(fullstr, strlen(fullstr)), 
			g_ascii_strdown(search, strlen(search))
		)!= NULL
	);
}

/* qsort C-string comparison function (with void* args) */ 
int cstring_cmp(const void *a, const void *b) 
{ 
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
} 

void bimp_write_manipulation(manipulation man, gpointer file) 
{
	fwrite(man, sizeof(struct manip_str), 1, file);
	fflush(file);
}

GimpParamDef bimp_get_param_info(gchar* proc_name, gint arg_num) 
{
	GimpParamDef param_info;
	GimpPDBArgType type;
	gchar *name;
	gchar *desc;
		
	gimp_procedural_db_proc_arg (
		proc_name,
		arg_num,
		&type,
		&name,
		&desc
	);
	
	param_info.type = type;
	param_info.name = g_strdup(name);
	param_info.description = g_strdup(desc);
	
	return param_info;
}
