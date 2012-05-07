/*
 * Functions used to build and return the main BIMP user interface
 */

#include <string.h>
#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include "bimp.h"
#include "bimp-manipulations.h"
#include "bimp-gui.h"
#include "bimp-manipulations-gui.h"
#include "bimp-operate.h"
#include "bimp-icons.h"

static GtkWidget* sequence_panel_new(void);
static GtkWidget* option_panel_new(void);
static void init_fileview(void);
static void add_to_fileview(char*);
static void refresh_fileview(void);
static void open_file_chooser(GtkWidget*, gpointer);
static void add_input_file(char*);
static void remove_input_file(GtkWidget*, GtkTreeSelection*);
static void remove_all_input_files(GtkWidget*, gpointer);
static void select_filename (GtkTreeView*, gpointer);
static void update_preview(char*);
static void set_output_folder(GtkWidget*, gpointer);
static void toggle_alteroverwrite(GtkWidget*, gpointer);
static void popmenus_init(void);
static void open_popup_menu(GtkWidget*, gpointer);
static void add_manipulation_from_id(GtkMenuItem*, gpointer);
static void edit_clicked_manipulation(GtkMenuItem*, gpointer);
static void remove_clicked_manipulation(GtkMenuItem*, gpointer);
static void add_manipulation_button(manipulation);
static void open_about();
static const gchar* progressbar_init_hidden (void);
static void progressbar_start_hidden(const gchar*, gboolean, gpointer);
static void progressbar_end_hidden (gpointer);
static void progressbar_settext_hidden (const gchar*, gpointer);
static void progressbar_setvalue_hidden (double, gpointer);

GtkWidget *panel_sequence;
GtkWidget *panel_options;
GtkWidget *hbox_sequence;
GtkWidget *scroll_sequence;
GtkWidget *popmenu_add;
GtkWidget *popmenu_edit;
GtkWidget *check_alertoverwrite;
GtkWidget *treeview_files;
GtkWidget *file_chooser;
GtkWidget *file_preview;

GtkWidget* progressbar_visible;
const gchar* progressbar_data;

enum /* TreeView stuff... */
{
  LIST_ITEM = 0,
  N_COLUMNS
};

manipulation clicked_man; /* temporary manipulation, selected by clicking on panel_seq buttons */

void bimp_show_gui() 
{
	GtkWidget* vbox_main;
	
	gimp_ui_init (PLUG_IN_BINARY, FALSE);
	
	bimp_window_main = gimp_dialog_new(
		PLUG_IN_FULLNAME,
		PLUG_IN_BINARY,
		NULL,
		0,
		NULL,
		NULL,
		GTK_STOCK_ABOUT, GTK_RESPONSE_HELP,
		GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL,
        GTK_STOCK_APPLY, GTK_RESPONSE_APPLY, NULL
	);
	
	gimp_window_set_transient (GTK_WINDOW(bimp_window_main));
	gtk_widget_set_size_request (bimp_window_main, MAIN_WINDOW_W, MAIN_WINDOW_H);
	gtk_window_set_resizable (GTK_WINDOW(bimp_window_main), FALSE);
	gtk_window_set_position(GTK_WINDOW(bimp_window_main), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(bimp_window_main), 5);
	
	vbox_main = gtk_vbox_new(FALSE, 10);
	panel_sequence = sequence_panel_new();
	panel_options = option_panel_new();
	
	progressbar_visible = gtk_progress_bar_new();
	gtk_widget_set_size_request (progressbar_visible, PROGRESSBAR_W, PROGRESSBAR_H);
	progressbar_data = progressbar_init_hidden();
	
	gtk_box_pack_start(GTK_BOX(vbox_main), panel_sequence, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_main), panel_options, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_main), progressbar_visible, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(bimp_window_main)->vbox), vbox_main);
	gtk_widget_show_all(bimp_window_main);
	
	while(TRUE) {
		gint run = gimp_dialog_run (GIMP_DIALOG(bimp_window_main));
		if (run == GTK_RESPONSE_APPLY) {
			if (g_slist_length(bimp_selected_manipulations) == 0) {
				char* error_message = g_strconcat("Manipulations list is empty!", NULL);
				bimp_show_error_dialog(error_message, bimp_window_main);
				g_free (error_message);
			} else {
				if (g_slist_length(bimp_input_filenames) == 0) {
					char* error_message = g_strconcat("Input files list is empty!", NULL);
					bimp_show_error_dialog(error_message, bimp_window_main);
					g_free (error_message);
				}
				else {
					gboolean success = bimp_start_batch(bimp_window_main);
				}
			}
		}
		else if (run == GTK_RESPONSE_HELP) {
			open_about();
		}
		else {
			gimp_progress_uninstall(progressbar_data);
			gtk_widget_destroy (bimp_window_main);
			return;
		}
	}
}

/* builds and returns the upper panel with the manipulation buttons */
static GtkWidget* sequence_panel_new()
{
	GtkWidget *panel;
	
	panel = gtk_frame_new("Manipulation steps");
	gtk_widget_set_size_request (panel, SEQ_PANEL_W, SEQ_PANEL_H);
	
	scroll_sequence = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_sequence), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
	
	hbox_sequence = gtk_hbox_new(FALSE, 10);
	
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll_sequence), hbox_sequence);
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(gtk_bin_get_child(GTK_BIN(scroll_sequence))), GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(panel), scroll_sequence);
	
	bimp_refresh_sequence_panel();
	popmenus_init();
	
	return panel;
}

/* builds and returns the panel with file list and options */
static GtkWidget* option_panel_new()
{
	GtkWidget *panel, *hbox;
	GtkWidget *hbox_buttons;
	GtkWidget *vbox_input;
	GtkWidget *scroll_input;
	GtkWidget *button_add, *button_remove, *button_removeall;
	
	GtkTreeSelection *treesel_file;
	
	GtkWidget *vbox_useroptions, *hbox_outfolder;
	GtkWidget *label_chooser, *button_outfolder;
	
	panel = gtk_frame_new("Input files and options");
	gtk_widget_set_size_request (panel, OPTION_PANEL_W, OPTION_PANEL_H);
	hbox = gtk_hbox_new(FALSE, 5);
	
	/* Sub-panel for input file listing and buttons */
	vbox_input = gtk_vbox_new(FALSE, 1);
	gtk_widget_set_size_request(vbox_input, INPUT_PANEL_W, INPUT_PANEL_H);
	
	/* Sub-sub-panel for input file listing */
	scroll_input = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_input), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(scroll_input, FILE_LIST_PANEL_W, FILE_LIST_PANEL_H);
	treeview_files = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview_files), FALSE);
	init_fileview();
	refresh_fileview();
	treesel_file = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_files));
	
	/* Sub-sub-panel for input file buttons */
	hbox_buttons = gtk_hbox_new(FALSE, 1);
	gtk_widget_set_size_request(hbox_buttons, FILE_LIST_BUTTONS_PANEL_W, FILE_LIST_BUTTONS_PANEL_H);
	
	button_add = gtk_button_new_with_label("Add image files");
	gtk_widget_set_size_request(button_add, FILE_LIST_BUTTON_W, FILE_LIST_BUTTON_H);
	button_remove = gtk_button_new_with_label("Remove selected");
	gtk_widget_set_size_request(button_remove, FILE_LIST_BUTTON_W, FILE_LIST_BUTTON_H);
	button_removeall = gtk_button_new_with_label("Remove all");
	gtk_widget_set_size_request(button_removeall, FILE_LIST_BUTTON_W, FILE_LIST_BUTTON_H);
	
	/* Sub-panel for options */
	vbox_useroptions = gtk_vbox_new(FALSE, 3);
	gtk_widget_set_size_request(vbox_useroptions, USEROPTIONS_PANEL_W, USEROPTIONS_PANEL_H);
	
	hbox_outfolder = gtk_hbox_new(FALSE, 3);
	label_chooser = gtk_label_new("Output folder:");
	button_outfolder = gtk_file_chooser_button_new("Select output folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	bimp_output_folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(button_outfolder));
	gtk_widget_set_size_request(button_outfolder, 170, 30);
	
	bimp_alertoverwrite = TRUE;
	check_alertoverwrite = gtk_check_button_new_with_label("Alert when overwriting existing files");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_alertoverwrite), bimp_alertoverwrite);
	
	file_preview = gtk_image_new_from_image(NULL, NULL);
	gtk_widget_set_size_request(file_preview, FILE_PREVIEW_W, FILE_PREVIEW_H);
	
	/* All together */
	gtk_box_pack_start(GTK_BOX(hbox_buttons), button_add, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_buttons), button_remove, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_buttons), button_removeall, FALSE, FALSE, 0);
	
	gtk_container_add (GTK_CONTAINER(scroll_input), treeview_files);
		
	gtk_box_pack_start(GTK_BOX(vbox_input), scroll_input, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_input), hbox_buttons, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(hbox_outfolder), label_chooser, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_outfolder), button_outfolder, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(vbox_useroptions), hbox_outfolder, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_useroptions), check_alertoverwrite, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox_useroptions), file_preview, FALSE, FALSE, 5);
	
	gtk_box_pack_start(GTK_BOX(hbox), vbox_input, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox_useroptions, FALSE, FALSE, 10);
	
	gtk_container_add(GTK_CONTAINER(panel), hbox);

	g_signal_connect(G_OBJECT(button_add), "clicked", G_CALLBACK(open_file_chooser), NULL);
	g_signal_connect(G_OBJECT(button_remove), "clicked", G_CALLBACK(remove_input_file), treesel_file);
	g_signal_connect(G_OBJECT(button_removeall), "clicked", G_CALLBACK(remove_all_input_files), NULL);
	g_signal_connect(G_OBJECT(button_outfolder), "selection-changed", G_CALLBACK(set_output_folder), NULL);
	g_signal_connect(G_OBJECT(check_alertoverwrite), "toggled", G_CALLBACK(toggle_alteroverwrite), NULL);
	g_signal_connect(G_OBJECT(treeview_files), "cursor-changed", G_CALLBACK(select_filename), treesel_file);
	
	return panel;
}

/* following: functions that modify the file list widget (add/remove/removeall)  */

static void add_input_file(char* filename) 
{
	if (g_slist_find_custom(bimp_input_filenames, filename, (GCompareFunc)strcmp) == NULL) {
		bimp_input_filenames = g_slist_append(bimp_input_filenames, filename);
		refresh_fileview();
	}
}

static void remove_input_file(GtkWidget *widget, GtkTreeSelection* selection) 
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	char *selected;
	
	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection), &model, &iter)) {
		gtk_tree_model_get(model, &iter, LIST_ITEM, &selected,  -1);
		bimp_input_filenames = g_slist_delete_link(bimp_input_filenames, g_slist_find_custom(bimp_input_filenames, selected, (GCompareFunc)strcmp));
		refresh_fileview();
		update_preview(NULL); /* clear the preview widget */
		g_free(selected);
	}
}

static void remove_all_input_files(GtkWidget *widget, gpointer selection) 
{
	g_slist_free(bimp_input_filenames);
	bimp_input_filenames = NULL;
	refresh_fileview();
	update_preview(NULL);
}

/* called when the user clicks on a filename row to update the preview widget */
static void select_filename (GtkTreeView *tree_view, gpointer selection) 
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	char *selected;
	GdkPixbuf *pixbuf_prev;         
	
	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection), &model, &iter)) {
		gtk_tree_model_get(model, &iter, LIST_ITEM, &selected,  -1);
		update_preview(selected);
	}
}

/* updates the preview widget loading (and scaling) a new image */
static void update_preview (gchar* filename) 
{
	GdkPixbuf *pixbuf_prev;         
	
	if (filename != NULL) {
		pixbuf_prev = gdk_pixbuf_new_from_file_at_scale(filename, FILE_PREVIEW_W, FILE_PREVIEW_H, TRUE, NULL);
		gtk_image_set_from_pixbuf(GTK_IMAGE(file_preview), pixbuf_prev);
	} else {
		gtk_image_set_from_pixbuf(GTK_IMAGE(file_preview), NULL); /* if filename is NULL, shows an "empty image" */
	}
}

static void init_fileview()
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkListStore *store;

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(
		"List Items",
		renderer, 
		"text", 
		LIST_ITEM, 
		NULL
	);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_files), column);

	store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_files), GTK_TREE_MODEL(store));
	g_object_unref(store);
}

static void add_to_fileview(char *str)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview_files)));
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, LIST_ITEM, str, -1);
}

/* update the visual of filename list */
static void refresh_fileview() 
{	
	GtkListStore *store;
	GtkTreeModel *model;
	GtkTreeIter  treeiter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (treeview_files)));
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview_files));

	/* clear all rows in list */
	if (gtk_tree_model_get_iter_first(model, &treeiter) == TRUE) {
		gtk_list_store_clear(store);
	}
	
	GSList *iter;
	for (iter = bimp_input_filenames; iter; iter = iter->next) {
        add_to_fileview(iter->data);
    }
}

static void open_file_chooser(GtkWidget *widget, gpointer data) 
{
	GSList *selection;
	
	if (file_chooser == NULL) { /* set the file chooser for the first time */
		GtkFileFilter *filter_all, *supported[5];

		file_chooser = gtk_file_chooser_dialog_new(
			"Select images", 
			NULL, 
			GTK_FILE_CHOOSER_ACTION_OPEN, 
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
			GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT, NULL
		);
		gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser), TRUE);
		
		filter_all = gtk_file_filter_new();
		gtk_file_filter_set_name(filter_all,"All supported images");
		
		supported[0] = gtk_file_filter_new();
		gtk_file_filter_set_name(supported[0],"BMP");
		gtk_file_filter_add_mime_type(supported[0],"image/bmp");
		gtk_file_filter_add_mime_type(filter_all,"image/bmp");
		
		supported[1] = gtk_file_filter_new();
		gtk_file_filter_set_name(supported[1],"JPEG");
		gtk_file_filter_add_mime_type(supported[1],"image/jpeg");
		gtk_file_filter_add_mime_type(filter_all,"image/jpeg");
		
		supported[2] = gtk_file_filter_new();
		gtk_file_filter_set_name(supported[2],"GIF");
		gtk_file_filter_add_mime_type(supported[2],"image/gif");
		gtk_file_filter_add_mime_type(filter_all,"image/gif");
		
		supported[3] = gtk_file_filter_new();
		gtk_file_filter_set_name(supported[3],"PNG");
		gtk_file_filter_add_mime_type(supported[3],"image/png");
		gtk_file_filter_add_mime_type(filter_all,"image/png");
		
		supported[4] = gtk_file_filter_new();
		gtk_file_filter_set_name(supported[4],"TIFF");
		gtk_file_filter_add_mime_type(supported[4],"image/tiff");
		gtk_file_filter_add_mime_type(filter_all,"image/tiff");
		
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), filter_all);
		int i;
		for(i = 0; i < 5; i++) {
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), supported[i]);
		}
	}
	
	/* show dialog */
	if (gtk_dialog_run (GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT) {
		selection = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(file_chooser));
		g_slist_foreach(selection, (GFunc)add_input_file, NULL);
	}
	
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(file_chooser));
	gtk_widget_hide (file_chooser);
}

static void set_output_folder(GtkWidget *widget, gpointer data) 
{
	bimp_output_folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
}

static void toggle_alteroverwrite(GtkWidget *widget, gpointer data) 
{
	bimp_alertoverwrite = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_alertoverwrite));
}

/* initializes the two menus that appears when the user clicks on the "add new" button 
 * or on one of the already added manipulations */
static void popmenus_init() 
{
	GtkWidget *menuitem;
	
	/* Menu for adding a manipulation */
	popmenu_add = gtk_menu_new();
	
	int man_id;
	for (man_id = 0; man_id < MANIP_END; man_id++)
	{
		menuitem = gtk_menu_item_new_with_label(manipulation_type_string[man_id]);
		g_signal_connect(menuitem, "activate", G_CALLBACK(add_manipulation_from_id), GINT_TO_POINTER(man_id));
		gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_add), menuitem);
	}
	
	/* Menu for editing a manipulation */
	popmenu_edit = gtk_menu_new();
	
	menuitem = gtk_menu_item_new_with_label("<step name>"); /* first element shows only the step name */
	gtk_widget_set_sensitive(menuitem, FALSE); /* and it's non-selectable */
	
	gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_edit), menuitem);
	menuitem = gtk_menu_item_new_with_label("Edit properties");
	g_signal_connect(menuitem, "activate", G_CALLBACK(edit_clicked_manipulation), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_edit), menuitem);
	menuitem = gtk_menu_item_new_with_label("Remove manipulation");
	g_signal_connect(menuitem, "activate", G_CALLBACK(remove_clicked_manipulation), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_edit), menuitem);
	
	gtk_widget_show_all(popmenu_add);
	gtk_widget_show_all(popmenu_edit);
}

static void open_popup_menu(GtkWidget *widget, gpointer data)
{
	if (data == NULL) {
		gtk_menu_popup(GTK_MENU(popmenu_add), NULL, NULL, NULL, NULL, 0, 0);
	}
	else {
		const gchar* item_label;
		
		clicked_man = (manipulation)data;
		if (clicked_man->type == MANIP_USERDEF) {
			item_label = ((userdef_settings)(clicked_man->settings))->procedure;
		} else {
			item_label = manipulation_type_string[clicked_man->type];
		}
		gtk_menu_item_set_label(g_list_first(gtk_container_get_children(GTK_CONTAINER(popmenu_edit)))->data, item_label);
		gtk_menu_popup(GTK_MENU(popmenu_edit), NULL, NULL, NULL, NULL, 0, 0);
	}
}

static void add_manipulation_from_id(GtkMenuItem *menuitem, gpointer id) 
{	
	int man_id = GPOINTER_TO_INT(id);
	manipulation newman = bimp_append_manipulation((manipulation_type)man_id);
	if (newman == NULL) {
		char* error_message = g_strconcat("Can't add another \"", manipulation_type_string[man_id], "\" step. \nOnly one is permitted!", NULL);
		bimp_show_error_dialog(error_message, bimp_window_main);
		g_free(error_message);
	}
	else {
		bimp_refresh_sequence_panel();
	
		GtkAdjustment* hadj_sequence = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scroll_sequence));
		gtk_adjustment_set_value(hadj_sequence, gtk_adjustment_get_upper(hadj_sequence));
		
		bimp_open_editwindow(newman, TRUE);
	}
}

static void edit_clicked_manipulation(GtkMenuItem *menuitem, gpointer user_data)
{
	if (clicked_man != NULL) {
		bimp_open_editwindow(clicked_man, FALSE);
	}
}

static void remove_clicked_manipulation(GtkMenuItem *menuitem, gpointer user_data) 
{
	if (clicked_man != NULL) {
		bimp_remove_manipulation(clicked_man);
		g_free(clicked_man);
		bimp_refresh_sequence_panel();
	}
}

/* update the visual of the sequence panel */
void bimp_refresh_sequence_panel()
{
	GtkWidget* button;
	
	/* Remove all buttons */
	g_list_foreach(gtk_container_get_children(GTK_CONTAINER(hbox_sequence)), (GFunc)gtk_widget_destroy, NULL);

	/* Rebuild panel */
	g_slist_foreach(bimp_selected_manipulations, (GFunc)add_manipulation_button, NULL);
	
	button = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_button_set_image_position(GTK_BUTTON(button), GTK_POS_TOP);
	gtk_widget_set_size_request (button, SEQ_BUTTON_W - 20, SEQ_BUTTON_H);
	gtk_box_pack_start(GTK_BOX(hbox_sequence), button, FALSE, FALSE, 3);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(open_popup_menu), NULL);
	
	gtk_widget_show_all(hbox_sequence);
}

/* creates a manipulation button and appends it to the sequence box */
static void add_manipulation_button(manipulation man) 
{
	GtkWidget* button;
	
	button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_pixbuf(gdk_pixbuf_from_pixdata(man->icon, TRUE, NULL)));
	gtk_button_set_image_position(GTK_BUTTON(button), GTK_POS_TOP);
	gtk_widget_set_size_request(button, SEQ_BUTTON_W, SEQ_BUTTON_H);
	gtk_box_pack_start(GTK_BOX(hbox_sequence), button, FALSE, FALSE, 3);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(open_popup_menu), man);
}

static void open_about() 
{
	GtkWidget *about;
	
	about = gtk_about_dialog_new();
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about), PLUG_IN_FULLNAME);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about), PLUG_IN_DESCRIPTION);
	char version_number[6];
	sprintf(version_number, "%d.%d", PLUG_IN_VERSION_MAJ, PLUG_IN_VERSION_MIN);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about), version_number);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about), "(C) 2012 - Alessandro Francesconi");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about), "http://www.alessandrofrancesconi.it/projects/bimp");
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about), gdk_pixbuf_from_pixdata(&pixdata_bimpicon, FALSE, NULL));
	gtk_dialog_run(GTK_DIALOG(about));
	gtk_widget_destroy(about);
}

/* shows an error dialog with a custom message */
void bimp_show_error_dialog(char* message, GtkWidget* parent) {
	GtkWidget* dialog = gtk_message_dialog_new (
		GTK_WINDOW(parent),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_OK,
		message
	);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

/* suppress progress popup by installing progress handlers that do nothing */
static const gchar* progressbar_init_hidden ()
{	
	GimpProgressVtable vtable = { 0, };

	vtable.start     = progressbar_start_hidden;
	vtable.end       = progressbar_end_hidden;
	vtable.set_text  = progressbar_settext_hidden;
	vtable.set_value = progressbar_setvalue_hidden;
  
	return gimp_progress_install_vtable (&vtable, NULL);
}
static void progressbar_start_hidden (const gchar *message, gboolean cancelable, gpointer user_data) { }
static void progressbar_end_hidden (gpointer user_data) { }
static void progressbar_settext_hidden (const gchar *message, gpointer user_data) { }
static void progressbar_setvalue_hidden (double percent, gpointer user_data) { }

void bimp_progress_bar_set(double fraction, char* text) {
	while (g_main_context_iteration(NULL, FALSE));
	if (fraction > 1.0) fraction = 1.0;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar_visible), fraction);
	if (text != NULL) {
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar_visible), text);
	}
}

void bimp_set_busy(gboolean busy) {
	while (g_main_context_iteration(NULL, FALSE));
	gtk_dialog_set_response_sensitive (GTK_DIALOG(bimp_window_main), GTK_RESPONSE_CANCEL, !busy);
	gtk_dialog_set_response_sensitive (GTK_DIALOG(bimp_window_main), GTK_RESPONSE_APPLY, !busy);
	gtk_widget_set_sensitive(panel_sequence, !busy);
	gtk_widget_set_sensitive(panel_options, !busy);
}
