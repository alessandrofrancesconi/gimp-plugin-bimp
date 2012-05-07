#include <string.h>
#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include "gui-userdef.h"
#include "../bimp.h"
#include "../bimp-gui.h"
#include "../bimp-manipulations.h"

static void show_proc_params(userdef_settings, GtkWidget*);
static void set_unselected(GtkWidget*);
static void build_temp_settings(gchar*);
static void open_proc_browser(GtkButton*, gpointer);
static void fill_frame(GtkWidget*);
static GimpParamDef get_param_info(gchar*, gint);

GtkWidget *frame_scroll, *frame_params;
GtkWidget *button_openbrowser;
GtkWidget *proc_browser;

userdef_settings temp_settings;
GtkWidget **param_widget; /* array of showable widgets for customizing the selected procedure */

GtkWidget* bimp_userdef_gui_new(userdef_settings settings, GtkWidget* parent)
{
	GtkWidget *gui;
	GtkWidget *label_help;
	
	gui = gtk_vbox_new(FALSE, 5);
	
	proc_browser = NULL;
	
	label_help = gtk_label_new(NULL);
	gtk_widget_set_size_request (label_help, LABEL_HELP_W, LABEL_HELP_H);
	gtk_label_set_markup (GTK_LABEL(label_help), 
		"Choose a GIMP procedure and define its parameters.\n"
		"System's procedures can't be used, and other ones \n"
		"could produce side effects. <b>Use at your own risk!</b>"
	);
	gtk_label_set_justify(GTK_LABEL(label_help), GTK_JUSTIFY_CENTER);
	
	button_openbrowser = gtk_button_new();
	gtk_widget_set_size_request (button_openbrowser, BUTTON_CHOOSE_W, BUTTON_CHOOSE_H);
	
	frame_params = gtk_frame_new("Procedure parameters:");
	gtk_widget_set_size_request (frame_params, FRAME_PARAM_W, FRAME_PARAM_H);
	
	gtk_box_pack_start(GTK_BOX(gui), label_help, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), button_openbrowser, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), frame_params, FALSE, FALSE, 0);
	
	if (settings->procedure != NULL) {
		/* copy user setting into temp_settings */
		temp_settings = (userdef_settings)g_malloc(sizeof(struct manip_userdef_set));
		
		temp_settings->procedure = g_strdup(settings->procedure);
		temp_settings->num_params = settings->num_params;
		
		temp_settings->params = g_new(GimpParam, settings->num_params);
		temp_settings->params_names = (gchar**)g_malloc(sizeof(gchar*) * settings->num_params);
		
		int param_i;
		for (param_i = 0; param_i < temp_settings->num_params; param_i++) {
			temp_settings->params_names[param_i] = g_strdup(settings->params_names[param_i]);
			
			temp_settings->params[param_i].type = settings->params[param_i].type;
			temp_settings->params[param_i].data = settings->params[param_i].data;
		}
	}
	show_proc_params(settings, parent);
	
	g_signal_connect(G_OBJECT(button_openbrowser), "clicked", G_CALLBACK(open_proc_browser), parent);
	
	return gui;
}

/* Reads given settings and prepares the input widgets to show on the frame */
static void show_proc_params(userdef_settings settings, GtkWidget* parent) 
{
	if (settings->procedure == NULL) {
		set_unselected(parent);
	} else {
		GtkWidget *vbox_params, *hbox_paramrow, *label_widget_desc;
		GimpParamDef param_info;
		GdkColor usercolor;
		gboolean show, supported = TRUE;

		g_free(param_widget);
		param_widget = g_new(GtkWidget*, settings->num_params);
		vbox_params = gtk_vbox_new(FALSE, 5);
        
		int param_i, show_count = 0;
		for (param_i = 0; param_i < settings->num_params && supported; param_i++) {
			param_info = get_param_info(settings->procedure, param_i);
			show = TRUE;
			switch(param_info.type) {
				/* case: this param is a number, prepare a gtk_spin_button */
				case GIMP_PDB_INT32:
					if (strcmp(param_info.name, "run-mode") == 0) {
						show = FALSE;
					}
					else {
						if (strcmp(param_info.name, "toggle") == 0) { /* TODO: search "true/false" in description? */
							/* but if it's named 'toggle' pretend to be a boolean value */
							param_widget[param_i] = gtk_check_button_new();
							gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(param_widget[param_i]), (settings->params[param_i]).data.d_int32 == 1 ? TRUE : FALSE);
						} else {
							param_widget[param_i] = gtk_spin_button_new (NULL, 1, 0);
							gtk_spin_button_configure (GTK_SPIN_BUTTON(param_widget[param_i]), GTK_ADJUSTMENT(gtk_adjustment_new ((settings->params[param_i]).data.d_int32, -10000, 10000, 1, 1, 0)), 0, 0);
						}	
					}
					break;
					
				case GIMP_PDB_INT16:
					param_widget[param_i] = gtk_spin_button_new (NULL, 1, 0);
					gtk_spin_button_configure (GTK_SPIN_BUTTON(param_widget[param_i]), GTK_ADJUSTMENT(gtk_adjustment_new ((settings->params[param_i]).data.d_int16, -10000, 10000, 1, 1, 0)), 0, 0);
					break;
					
				case GIMP_PDB_INT8:
					param_widget[param_i] = gtk_spin_button_new (NULL, 1, 0);
					gtk_spin_button_configure (GTK_SPIN_BUTTON(param_widget[param_i]), GTK_ADJUSTMENT(gtk_adjustment_new ((settings->params[param_i]).data.d_int8, 0, 255, 1, 1, 0)), 0, 0);
					break;
					
				case GIMP_PDB_FLOAT: 
					param_widget[param_i] = gtk_spin_button_new (NULL, 1, 1);
					if (strcmp(param_info.name, "opacity") == 0) {
						/* if the param is an opacity, its range goes from 0.0 to 100.0 */
						gtk_spin_button_configure (GTK_SPIN_BUTTON(param_widget[param_i]), GTK_ADJUSTMENT(gtk_adjustment_new ((float)((settings->params[param_i]).data.d_float), 0.0, 100.0, 0.1, 1, 0)), 0, 1);
					} else {
						gtk_spin_button_configure (GTK_SPIN_BUTTON(param_widget[param_i]), GTK_ADJUSTMENT(gtk_adjustment_new ((float)((settings->params[param_i]).data.d_float), -10000.0, 10000.0, 0.1, 1, 0)), 0, 1);
					}
					break;
					
				/* case: this param is a string, prepare a gtk_entry */
				case GIMP_PDB_STRING: 
					if (
						(strcmp(param_info.name, "filename") == 0) ||
						(strcmp(param_info.name, "raw-filename") == 0)
					) {
						supported = FALSE;
						show = FALSE;
					}
					else if (strcmp(param_info.name, "font") == 0){
						param_widget[param_i] = gtk_font_button_new_with_font((settings->params[param_i]).data.d_string);
					}
					else {
						param_widget[param_i] = gtk_entry_new();
						gtk_entry_set_text (GTK_ENTRY(param_widget[param_i]), (settings->params[param_i]).data.d_string);
					}
					break;
				
				/* case: this param is a color, prepare a color chooser */
				case GIMP_PDB_COLOR: 
					usercolor.red = (guint16)(((settings->params[param_i]).data.d_color.r)*65535);
					usercolor.green = (guint16)(((settings->params[param_i]).data.d_color.g)*65535);
					usercolor.blue = (guint16)(((settings->params[param_i]).data.d_color.b)*65535);
					param_widget[param_i] = gtk_color_button_new_with_color(&usercolor);
					break;
					
				case GIMP_PDB_DRAWABLE:
				case GIMP_PDB_IMAGE:
					show = FALSE; 
					break;
					
				/* case: this param is not (yet) supported (like arrays, paths, ...), can't continue */
				default: 
					supported = FALSE;
					show = FALSE; 
					break;
			}
			
			if (show) {
				gtk_widget_set_size_request (param_widget[param_i], PARAM_WIDGET_W, PARAM_WIDGET_H);
				label_widget_desc = gtk_label_new(param_info.description);
				gtk_widget_set_tooltip_text (label_widget_desc, param_info.name);
				gtk_misc_set_alignment(GTK_MISC(label_widget_desc), 0, 0.5);
				
				hbox_paramrow = gtk_hbox_new(FALSE, 5);
				gtk_box_pack_start(GTK_BOX(hbox_paramrow), param_widget[param_i], FALSE, FALSE, 0);
				gtk_box_pack_start(GTK_BOX(hbox_paramrow), label_widget_desc, FALSE, FALSE, 0);
				gtk_box_pack_start(GTK_BOX(vbox_params), hbox_paramrow, FALSE, FALSE, 0);
				
				show_count++;
			} else {
				param_widget[param_i] = NULL; /* this param is not editable, so there is no visualizable widget for him */
			}
		}
		
		if (supported) {
			gtk_button_set_label(GTK_BUTTON(button_openbrowser), g_strconcat(settings->procedure, " (click to change)", NULL));
			gtk_dialog_set_response_sensitive (GTK_DIALOG(parent), GTK_RESPONSE_ACCEPT, TRUE);
			
			if (show_count > 0) {
				fill_frame(vbox_params); /* send all the widgets to the filling function */
			}
			else {
				GtkWidget* label_noparams;
				label_noparams = gtk_label_new("This procedure takes no editable params");
				fill_frame(label_noparams);
			}
		} else {
			/* if a unsupported param has been found, the loop ends and a message is displayed */
			bimp_show_error_dialog("Selected procedure contains unsupported parameters", NULL);
			set_unselected(parent);
		}
	}
}

/* set the dialog in "no selected procedure" view */
static void set_unselected(GtkWidget* parent) 
{
	GtkWidget* label_noproc;
	gtk_button_set_label(GTK_BUTTON(button_openbrowser), "Choose procedure");
	label_noproc = gtk_label_new("Can't save because\nno procedure has been selected");
	gtk_label_set_justify(GTK_LABEL(label_noproc), GTK_JUSTIFY_CENTER);
	gtk_dialog_set_response_sensitive (GTK_DIALOG(parent), GTK_RESPONSE_ACCEPT, FALSE);
	fill_frame(label_noproc);
}

/* fills the frame with a GtkScrolledWindow containing the given widgets */
static void fill_frame(GtkWidget* widget) 
{	
	if (gtk_bin_get_child(GTK_BIN(frame_params)) != NULL) {
		gtk_widget_destroy(GTK_WIDGET(gtk_bin_get_child(GTK_BIN(frame_params))));
	}
	
	frame_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(frame_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	
	if (widget != NULL) {
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(frame_scroll), widget);
		gtk_viewport_set_shadow_type(GTK_VIEWPORT(gtk_bin_get_child(GTK_BIN(frame_scroll))), GTK_SHADOW_NONE);
	}
	
	gtk_container_add(GTK_CONTAINER(frame_params), frame_scroll);
	gtk_widget_show_all(frame_scroll);
}

/* build a temporary userdef_settings struct from a given procedure and filling it with default values */
static void build_temp_settings(gchar* procedure) 
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
		procedure,
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
	
	g_free(temp_settings);
	temp_settings = (userdef_settings)malloc(sizeof(struct manip_userdef_set));
	
	temp_settings->procedure = g_strdup(procedure);
	temp_settings->num_params = num_params;
	
	temp_settings->params = g_new(GimpParam, num_params);
	temp_settings->params_names = (gchar**)malloc(sizeof(gchar*) * num_params);
	
	GimpRGB tempcolor;
	gimp_rgb_set(&tempcolor, 0.0, 0.0, 0.0);
	
	int param_i;
	for(param_i = 0; param_i < temp_settings->num_params; param_i++) {
		temp_settings->params_names[param_i] = g_strdup(params[param_i].name);
		
		temp_settings->params[param_i].type = params[param_i].type;
		
		/* set default values */
		switch(temp_settings->params[param_i].type) {
			case GIMP_PDB_INT32:
				temp_settings->params[param_i].data.d_int32 = 0;
			case GIMP_PDB_INT16:
				temp_settings->params[param_i].data.d_int16 = 0;
			case GIMP_PDB_INT8:
				temp_settings->params[param_i].data.d_int8 = 0;
			case GIMP_PDB_FLOAT: 
				if (strcmp(temp_settings->params_names[param_i], "opacity") == 0) {
					temp_settings->params[param_i].data.d_float = 100.0;
				} else {
					temp_settings->params[param_i].data.d_float = 0.0;
				}
				break;
			case GIMP_PDB_STRING: 
				if (strcmp(temp_settings->params_names[param_i], "font") == 0) {
					temp_settings->params[param_i].data.d_string = "Sans 16px";
				} else {
					temp_settings->params[param_i].data.d_string = "";
				}
				break;
			case GIMP_PDB_COLOR:
				temp_settings->params[param_i].data.d_color = tempcolor;
				break;
				
			default: 
				break;
		}
	}
}

/* open the GIMP procedure browser and */
static void open_proc_browser(GtkButton *button, gpointer parent)
{
	gint result;
	gchar* browser_selection;
	
	if (proc_browser == NULL) { /* set the procedure browser for the first time */
		proc_browser = gimp_proc_browser_dialog_new (
			"Procedure Browser",
			PLUG_IN_BINARY,
			gimp_standard_help_func, 
			PLUG_IN_PROC,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CLOSE, NULL
		);
	}
	
	if (gtk_dialog_run (GTK_DIALOG (proc_browser)) == GTK_RESPONSE_OK) {
		browser_selection = gimp_proc_browser_dialog_get_selected((GimpProcBrowserDialog*)proc_browser);	
		if (
			/* check if the selected procedure can be used */
			strstr(browser_selection, PLUG_IN_PROC) != NULL ||
			strstr(browser_selection, "-help") != NULL ||
			strstr(browser_selection, "-load") != NULL ||
			strstr(browser_selection, "-help") != NULL ||
			strstr(browser_selection, "-get-") != NULL || /* it might be excessive... */
			strstr(browser_selection, "-set-") != NULL ||
			strstr(browser_selection, "gimp-buffer") != NULL ||
			strstr(browser_selection, "gimp-online") != NULL ||
			strstr(browser_selection, "gimp-progress") != NULL ||
			strstr(browser_selection, "gimp-procedural") != NULL ||
			strstr(browser_selection, "gimp-context") != NULL ||
			strstr(browser_selection, "gimp-edit") != NULL ||
			strstr(browser_selection, "gimp-fonts") != NULL ||
			strstr(browser_selection, "gimp-palette") != NULL ||
			strstr(browser_selection, "gimp-layer") != NULL ||
			strstr(browser_selection, "gimp-vectors") != NULL
			) {
			bimp_show_error_dialog("You've selected an unsupported or invalid operation", NULL);
		} else {
			if (temp_settings == NULL || (temp_settings != NULL && strcmp(temp_settings->procedure, browser_selection) != 0)) {				
				build_temp_settings(browser_selection);
				show_proc_params(temp_settings, parent);
			}
		}
	}
	
	gtk_widget_hide (proc_browser); /* simply hide the browser so next time the cursor is in the latest position */
}

/* calls gimp_procedural_db_proc_arg to retrieve parameter's informations */
static GimpParamDef get_param_info(gchar* proc_name, gint arg_num) 
{
	GimpParamDef param_info;
	GimpPDBArgType type;
	gchar *name;
	gchar *desc;
	
	gboolean success;
	
	success = gimp_procedural_db_proc_arg (
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

/* store all the data from temp settings into the user settings */
void bimp_userdef_save(userdef_settings orig_settings) 
{
	g_free(orig_settings->procedure);
	
	orig_settings->procedure = g_strdup(temp_settings->procedure);
	orig_settings->num_params = temp_settings->num_params;
	
	g_free(orig_settings->params);
	g_free(orig_settings->params_names);
	orig_settings->params = g_new(GimpParam, orig_settings->num_params);
	orig_settings->params_names = (gchar**)malloc(sizeof(gchar*) * orig_settings->num_params);
	
	int param_i;
	for (param_i = 0; param_i < temp_settings->num_params; param_i++) {
		orig_settings->params_names[param_i] = g_strdup(temp_settings->params_names[param_i]);
		
		orig_settings->params[param_i].type = temp_settings->params[param_i].type;
		
		GdkColor usercolor;
		GimpRGB rgbdata;
		switch(orig_settings->params[param_i].type) {
			case GIMP_PDB_INT32:
				if (strcmp(temp_settings->params_names[param_i], "run-mode") == 0) {
					(orig_settings->params[param_i]).data.d_int32 = (gint32)GIMP_RUN_NONINTERACTIVE;
				} else if (strcmp(temp_settings->params_names[param_i], "toggle") == 0) {
					(orig_settings->params[param_i]).data.d_int32 = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(param_widget[param_i])) ? 1 : 0;
				}
				else {
					(orig_settings->params[param_i]).data.d_int32 = (gint32)gtk_spin_button_get_value(GTK_SPIN_BUTTON(param_widget[param_i]));
				}
				break;
				
			case GIMP_PDB_INT16:
				(orig_settings->params[param_i]).data.d_int16 = (gint16)gtk_spin_button_get_value(GTK_SPIN_BUTTON(param_widget[param_i]));
				break;
				
			case GIMP_PDB_INT8:
				(orig_settings->params[param_i]).data.d_int8 = (gint8)gtk_spin_button_get_value(GTK_SPIN_BUTTON(param_widget[param_i]));
				break;
				
			case GIMP_PDB_FLOAT: 
				(orig_settings->params[param_i]).data.d_float = (gdouble)gtk_spin_button_get_value(GTK_SPIN_BUTTON(param_widget[param_i]));
				break;
				
			case GIMP_PDB_STRING: 
				if (strcmp(temp_settings->params_names[param_i], "font") == 0) {
					(orig_settings->params[param_i]).data.d_string = g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(param_widget[param_i])));
				} else {
					(orig_settings->params[param_i]).data.d_string = g_strdup(gtk_entry_get_text(GTK_ENTRY(param_widget[param_i])));
				}
				break;
			
			case GIMP_PDB_COLOR: 
				gtk_color_button_get_color(GTK_COLOR_BUTTON(param_widget[param_i]), &usercolor);
				gimp_rgb_set(&rgbdata, (gdouble)usercolor.red/65535, (gdouble)usercolor.green/65535, (gdouble)usercolor.blue/65535);
				(orig_settings->params[param_i]).data.d_color = rgbdata;
				break;
				
			default: break;
		}
	}
	
	g_free(temp_settings);
}
