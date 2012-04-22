/*
 * BIMP - Batch Image Manipulation Plugin for GIMP
 * Version 0.1
 * (C)2012 - Alessandro Francesconi
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


#include <libgimp/gimp.h>
#include "bimp.h"
#include "bimp-manipulations.h"
#include "bimp-gui.h"

static void query (void);

static void run (
	const gchar *name,
	gint nparams,
	const GimpParam *param,
	gint *nreturn_vals,
	GimpParam **return_vals
	);

const GimpPlugInInfo PLUG_IN_INFO =
{
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
	
	gimp_install_procedure (
		PLUG_IN_PROC,
		PLUG_IN_FULLNAME,
		PLUG_IN_DESCRIPTION,
		"Alessandro Francesconi <alessandrofrancesconi@live.it>",
		"Copyright (C) Alessandro Francesconi\n"
		"http://www.alessandrofrancesconi.it/projects/bimp",
		"2012",
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

	values[0].type = GIMP_PDB_STATUS;
	values[0].data.d_status = status;

	run_mode = param[0].data.d_int32;
	switch (run_mode) {
		case GIMP_RUN_INTERACTIVE:
		case GIMP_RUN_WITH_LAST_VALS:
			bimp_show_gui();
			break;

		case GIMP_RUN_NONINTERACTIVE:
		default:
			g_error("Bimp can't run in non-interactive mode. At least for now...");
			values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
			break;
	}
}
