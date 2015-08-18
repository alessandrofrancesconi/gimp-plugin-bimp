/* shared utility functions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <libgimp/gimp.h>
#include "bimp-utils.h"

#ifdef __unix__
    #include <unistd.h>
#elif defined _WIN32
    #include <windows.h>
#elif defined __APPLE__
    #include <mach-o/dyld.h>
#endif

#include <sys/stat.h>
#include <time.h>
#include <utime.h>

/* replace all the occurrences of 'rep' into 'orig' with text 'with' */
char* str_replace(char *orig, char *rep, char *with) 
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
char* comp_get_filename(char* path) 
{
    char *pfile;
    
    pfile = path + strlen(path);
    for (; pfile > path; pfile--)
    {
        if ((*pfile == FILE_SEPARATOR)) //'\\') || (*pfile == '/'))
        {
            pfile++;
            break;
        }
    }

    return pfile;
}

/* gets only the file folder from the given path 
 * (compatible with unix and win) */
char* comp_get_filefolder(char* path) 
{
    int i;
    char *folder = strdup(path);

    for (i = strlen(folder); i > 0 ; i--)
    {
        if ((folder[i-1] == FILE_SEPARATOR))
        {
            folder[i] = '\0';
            break;
        }
    }
    return folder;
}

/* return TRUE if the first string 'fullstr' contains one or more occurences of substring 'search'.
 * (case-insensitive version) */
gboolean str_contains_cins(char* fullstr, char* search) {
    return (
        strstr(
            g_ascii_strdown(fullstr, strlen(fullstr)), 
            g_ascii_strdown(search, strlen(search))
        )!= NULL
    );
}

gboolean file_has_extension(char* file, char* ext) {
    return g_str_has_suffix(
        g_ascii_strdown(file, strlen(file)), 
        g_ascii_strdown(ext, strlen(ext))
    ); 
}

GimpParamDef pdb_proc_get_param_info(gchar* proc_name, gint arg_num) 
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

char* get_user_dir() 
{
    char* path = NULL;
    
#ifdef _WIN32
    path = g_strconcat(getenv("HOMEDRIVE"), getenv("HOMEPATH"), NULL);
    if (strlen(path) == 0) path = "C:\\";
#else
    path = getenv("HOME");
    if (strlen(path) == 0) path = "/";
#endif
    
    return path;
}

char* get_bimp_localedir() 
{
    int bufsize = 1024;
    char* path = g_malloc0(bufsize);
    
    // different methods for getting the plugin's absolute path, for different systems
#ifdef __unix__
    readlink("/proc/self/exe", path, bufsize);
#elif defined _WIN32
    GetModuleFileName(GetModuleHandle(NULL), path, bufsize);
#elif defined __APPLE__
    _NSGetExecutablePath(path, &bufsize);
#endif
    
    memset(g_strrstr(path, FILE_SEPARATOR_STR), '\0', 1); // truncate at the last path separator (eliminates "bimp.exe")
    
    return g_strconcat(path, FILE_SEPARATOR_STR, "bimp-locale", NULL); // returns truncated path, plus "/bimp-locale" directory
}

/* C-string case-insensitive comparison function (with gconstpointer args) */ 
int glib_strcmpi(gconstpointer str1, gconstpointer str2)
{
    return strcasecmp(str1, str2);
}

gchar** get_path_folders (char *path)
{
    char * normalized_path = (char*)g_malloc(sizeof(path));

    normalized_path = g_strdup(path);
    return g_strsplit(normalized_path, FILE_SEPARATOR_STR, 0);
}

/* gets the current date and time in "%Y-%m-%d_%H-%M" format */
char* get_datetime() 
{
    time_t rawtime;
    struct tm * timeinfo;
    char* format;

    format = (char*)malloc(sizeof(char)*18);
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    strftime (format, 18, "%Y-%m-%d_%H-%M", timeinfo);

    return format;
}

time_t get_modification_time(char* filename) 
{
    struct stat filestats;
    if (stat(filename, &filestats) < 0) {
        return -1;
    }

    return filestats.st_mtime;
}

int set_modification_time(char* filename, time_t mtime) 
{
    struct stat filestats;
    
    if (stat(filename, &filestats) < 0) {
        return -1;
    }

    struct utimbuf new_time;
    new_time.actime = filestats.st_atime;
    new_time.modtime = mtime;
    if (utime(filename, &new_time) < 0) {
        return -1;
    }
    else return 0;
}