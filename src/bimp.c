/*
 * BIMP - Batch Image Manipulation Plugin for GIMP
 * 
 * (C)2013 - Alessandro Francesconi
 * http://www.alessandrofrancesconi.it/projects/bimp
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */


#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <stdlib.h>
#include <string.h>
#include "bimp.h"
#include "bimp-manipulations.h"
#include "bimp-gui.h"
#include "bimp-utils.h"
#include "plugin-intl.h"

#ifdef __unix__
	#include <unistd.h>
#elif defined _WIN32
	#include <windows.h>
#elif defined __APPLE__
	#include <mach-o/dyld.h>
#endif

static void query (void);
static GSList* get_supported_procedures(void);
static gboolean proc_has_compatible_params (gchar*);
static char* get_bimp_localedir(void);
static int glib_strcmpi(gconstpointer, gconstpointer);

static void run (
	const gchar *name,
	gint nparams,
	const GimpParam *param,
	gint *nreturn_vals,
	GimpParam **return_vals
	);

const GimpPlugInInfo PLUG_IN_INFO = {
	NULL,  /* init_proc  */
	NULL,  /* quit_proc  */
	query, /* query_proc */
	run,   /* run_proc   */
};

MAIN ()

static void query (void)
{
	static GimpParamDef args[] = {
		{ GIMP_PDB_INT32, "run-mode", "Run mode" }
	};
	
	gimp_plugin_domain_register (GETTEXT_PACKAGE, get_bimp_localedir());

	gimp_install_procedure (
		PLUG_IN_PROC,
		PLUG_IN_FULLNAME,
		_("Applies GIMP manipulations on groups of images"),
		"Alessandro Francesconi <alessandrofrancesconi@live.it>",
		"Copyright (C) Alessandro Francesconi\n"
		"http://www.alessandrofrancesconi.it/projects/bimp",
		"2013",
		"Batch Image Manipulation...",
		"",
		GIMP_PLUGIN,
		G_N_ELEMENTS (args),
		0,
		args, 
		0
	);

	gimp_plugin_menu_register (PLUG_IN_PROC, "<Image>/File/Open"); 
}

static void run (
	const gchar *name,
	gint nparams,
	const GimpParam *param,
	gint *nreturn_vals,
	GimpParam **return_vals)
{
	static GimpParam  values[1];
	GimpPDBStatusType status = GIMP_PDB_SUCCESS;
	GimpRunMode run_mode;
	
	*nreturn_vals = 1;
	*return_vals  = values;

	bindtextdomain (GETTEXT_PACKAGE, get_bimp_localedir());
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
  
	values[0].type = GIMP_PDB_STATUS;
	values[0].data.d_status = status;
	
	run_mode = param[0].data.d_int32;
	
	switch (run_mode) {
		case GIMP_RUN_INTERACTIVE:
		case GIMP_RUN_WITH_LAST_VALS:
			bimp_supported_procedures = get_supported_procedures();
			bimp_show_gui();
			break;

		case GIMP_RUN_NONINTERACTIVE:
		default:
			g_error("Bimp can't run in non-interactive mode. At least for now...");
			values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
			break;
	}
}

/*
 * Used by userdef gui, filters the full list of GIMP procedures:
 *  - by ignoring system's procedures 
 *  - by ignoring procedures that contains non-compatible datatypes (like FLOATARRAY, PATH, ...). */
static GSList* get_supported_procedures()
{
	gint proc_count;
	gchar** results;
	GSList* compatible_list = NULL;
	
	gimp_procedural_db_query (
		"^(?!.*(?:"
			"plug-in-bimp|"
			"extension-|"
			"-get-|"
			"-set-|"
			"-is-|"
			"-has-|"
			"-get-|"
			"-print-|"
			"file-glob|"
			"twain-acquire|"
			"-load|"
			"-save|" // TODO: remove it for next feature "enable saving plugins"
			"-select|"
			"-free|"
			"-help|"
			"-temp|"
			"-undo|"
			"-copy|"
			"-paste|"
			"-cut|"
			"-channel|"
			"-buffer|"
			"-register|"
			"-metadata|"
			"-layer|"
			"-selection|"
			"-brush|"
			"-gradient|"
			"-guide|"
			"-parasite|"
			"gimp-online|"
			"gimp-progress|"
			"gimp-procedural|"
			"gimp-display|"
			"gimp-context|"
			"gimp-fonts|"
			"gimp-palette|"
			"gimp-path|"
			"gimp-pattern|"
			"gimp-vectors|"
			"gimp-quit|"
			"gimp-plugins|"
			"gimp-gimprc|"
			"temp-procedure"
		")).*",
		".*",
		".*",
		".*",
		".*",
		".*",
		".*",
		&proc_count,
		&results
	);
	
	int i;
	for (i = 0; i < proc_count; i++) {
		/* check each parameter for compatibility and sort it alphabetically */
		if (proc_has_compatible_params(results[i])) {
			//compatible_list = g_slist_append(compatible_list, results[i]);
			compatible_list = g_slist_insert_sorted(compatible_list, results[i], glib_strcmpi);
		}
	}
	
	free (results);
	
	return compatible_list;
}

static gboolean proc_has_compatible_params(gchar* proc_name) 
{
	gchar* proc_blurb;
	gchar* proc_help;
	gchar* proc_author;
	gchar* proc_copyright;
	gchar* proc_date;
	GimpPDBProcType proc_type;
	gint num_params;
	gint num_values;
	GimpParamDef *params;
	GimpParamDef *return_vals;
	
	gimp_procedural_db_proc_info (
		proc_name,
		&proc_blurb,
		&proc_help,
		&proc_author,
		&proc_copyright,
		&proc_date,
		&proc_type,
		&num_params,
		&num_values,
		&params,
		&return_vals
	);
	
	int i;
	GimpParamDef param;
	gboolean compatible = TRUE;
	for (i = 0; (i < num_params) && compatible; i++) {
		param = bimp_get_param_info(proc_name, i);
		
		if (
			param.type == GIMP_PDB_INT32 ||
			param.type == GIMP_PDB_INT16 ||
			param.type == GIMP_PDB_INT8 ||
			param.type == GIMP_PDB_FLOAT ||
			//(param.type == GIMP_PDB_STRING && strstr(param.name, "filename") == NULL) ||
			param.type == GIMP_PDB_STRING ||
			param.type == GIMP_PDB_COLOR ||
			param.type == GIMP_PDB_DRAWABLE ||
			param.type == GIMP_PDB_IMAGE
			) {
			compatible = TRUE;
		} else {
			compatible = FALSE;
		}
	}
	
	return (compatible && num_params > 0);
}

static char* get_bimp_localedir() 
{
	char path[1024];
	int bufsize = sizeof(path);
	char div[2];
	
	// different methods for getting the plugin's absolute path, for different systems
#ifdef __unix__
	readlink("/proc/self/exe", path, bufsize);
#elif defined _WIN32
	GetModuleFileName(GetModuleHandle(NULL), path, bufsize);
#elif defined __APPLE__
	_NSGetExecutablePath(path, &bufsize);
#endif
	
	div[0] = FILE_SEPARATOR /*'/'*/;
	div[1] = '\0';
	memset(g_strrstr(path, div), '\0', 1); // truncate at the last path divisor (eliminates "bimp.exe")
	
	return g_strconcat(path, div, "bimp-locale", NULL); // returns truncated path, plus "/bimp-locale" directory
}

/* C-string case-insensitive comparison function (with gconstpointer args) */ 
static int glib_strcmpi(gconstpointer str1, gconstpointer str2)
{
    return strcmpi(str1, str2);
}

