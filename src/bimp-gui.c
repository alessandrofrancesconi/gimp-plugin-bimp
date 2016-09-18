/*
 * Functions used to build and return the main BIMP user interface
 */

#include <gtk/gtk.h>
#include <glib.h>
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "bimp.h"
#include "bimp-manipulations.h"
#include "bimp-gui.h"
#include "bimp-manipulations-gui.h"
#include "bimp-operate.h"
#include "bimp-serialize.h"
#include "bimp-icons.h"
#include "bimp-utils.h"
#include "plugin-intl.h"

static GtkWidget* sequence_panel_new(void);
static GtkWidget* option_panel_new(void);
static void init_fileview(void);
static void add_to_fileview(char*);
static GSList* get_treeview_selection();
static void open_file_chooser(GtkWidget*, gpointer);
static void open_folder_chooser(GtkWidget*, gpointer);
static void add_input_file(char*);
static void add_input_folder(char*, gpointer); 
static void add_opened_files(void); 
static void remove_input_file(GtkWidget*, gpointer);
static void remove_all_input_files(GtkWidget*, gpointer);
static void select_filename (GtkTreeView*, gpointer);
static void update_selection(char*);
static void show_preview(GtkTreeView*, gpointer);
static void open_outputfolder_chooser(GtkWidget*, gpointer);
static void set_source_output_folder(GtkWidget*, gpointer);
static void popmenus_init(void);
static void open_manipulation_popupmenu(GtkWidget*, gpointer);
static void open_addfiles_popupmenu(GtkWidget*, gpointer);
static void open_removefiles_popupmenu(GtkWidget*, gpointer);
static void add_manipulation_from_id(GtkMenuItem*, gpointer);
static void edit_clicked_manipulation(GtkMenuItem*, gpointer);
static void remove_clicked_manipulation(GtkMenuItem*, gpointer);
static void add_manipulation_button(manipulation);

static void save_set(GtkMenuItem*, gpointer);
static void load_set(GtkMenuItem*, gpointer);

static void open_about();
static const gchar* progressbar_init_hidden (void);
static void progressbar_start_hidden(const gchar*, gboolean, gpointer);
static void progressbar_end_hidden (gpointer);
static void progressbar_settext_hidden (const gchar*, gpointer);
static void progressbar_setvalue_hidden (double, gpointer);

GtkWidget *panel_sequence, *panel_options;
GtkWidget *hbox_sequence;
GtkWidget *scroll_sequence;
GtkWidget *popmenu_add, *popmenu_edit, *popmenu_addfiles, *popmenu_removefiles;
GtkWidget *check_keepfolderhierarchy, *check_deleteondone, *check_keepdates;
GtkWidget *treeview_files;
GtkWidget *button_preview, *button_outfolder, *button_samefolder;
GtkWidget* progressbar_visible;

char* selected_source_folder;
char* last_input_location;
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
        GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
        GTK_STOCK_APPLY, GTK_RESPONSE_APPLY, 
        GTK_STOCK_STOP, GTK_RESPONSE_CANCEL, NULL
    );
    
    gimp_window_set_transient (GTK_WINDOW(bimp_window_main));
    gtk_widget_set_size_request (bimp_window_main, MAIN_WINDOW_W, MAIN_WINDOW_H);
    gtk_window_set_resizable (GTK_WINDOW(bimp_window_main), FALSE);
    gtk_window_set_position(GTK_WINDOW(bimp_window_main), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(bimp_window_main), 5);
    
    // Forces the visualization of label AND images on buttons (especially for Windows)
    GtkSettings *default_settings = gtk_settings_get_default();
    g_object_set(default_settings, "gtk-button-images", TRUE, NULL);
    
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
    gtk_widget_hide(button_preview);
    
    bimp_set_busy(FALSE);
    
    while(TRUE) {
        gint run = gimp_dialog_run (GIMP_DIALOG(bimp_window_main));
        if (run == GTK_RESPONSE_APPLY) {
            if (g_slist_length(bimp_selected_manipulations) == 0) {
                bimp_show_error_dialog(_("The manipulations set is empty!"), bimp_window_main);
            } else {
                if (g_slist_length(bimp_input_filenames) == 0) {
                    bimp_show_error_dialog(_("The file list is empty!"), bimp_window_main);
                }
                else {
                    bimp_opt_alertoverwrite = BIMP_ASK_OVERWRITE;
                    bimp_opt_keepfolderhierarchy = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_keepfolderhierarchy));
                    bimp_opt_deleteondone = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_deleteondone));
                    bimp_opt_keepdates = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_keepdates));
                    bimp_start_batch(bimp_window_main);
                }
            }
        }
        else if (run == GTK_RESPONSE_HELP) {
            open_about();
        }
        else if (run == GTK_RESPONSE_CANCEL) {
            bimp_set_busy(FALSE);
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
    
    panel = gtk_frame_new(_("Manipulation set"));
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
    GtkWidget *button_add, *button_remove;
    
    GtkWidget *vbox_useroptions, *hbox_outfolder;
    GtkWidget *label_chooser;
    
    panel = gtk_frame_new(_("Input files and options"));
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
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_files)), GTK_SELECTION_MULTIPLE);
    
    /* Sub-panel for input file buttons */
    
    hbox_buttons = gtk_hbox_new(FALSE, 1);
    gtk_widget_set_size_request(hbox_buttons, FILE_LIST_BUTTONS_PANEL_W, FILE_LIST_BUTTONS_PANEL_H);
    
    button_add = gtk_button_new_with_label(_("Add images"));
    gtk_widget_set_size_request(button_add, FILE_LIST_BUTTON_W, FILE_LIST_BUTTON_H);
    button_remove = gtk_button_new_with_label(_("Remove images"));
    gtk_widget_set_size_request(button_remove, FILE_LIST_BUTTON_W, FILE_LIST_BUTTON_H);
    
    /* Sub-panel for options */
    vbox_useroptions = gtk_vbox_new(FALSE, 3);
    gtk_widget_set_size_request(vbox_useroptions, USEROPTIONS_PANEL_W, USEROPTIONS_PANEL_H);
    
    hbox_outfolder = gtk_hbox_new(FALSE, 3);
    label_chooser = gtk_label_new(_("Output folder"));
    
    bimp_output_folder = get_user_dir();
    
    char* last_folder = g_strrstr(bimp_output_folder, FILE_SEPARATOR_STR) + 1;
    if (last_folder == NULL || strlen(last_folder) == 0) last_folder = bimp_output_folder;
    button_outfolder = gtk_button_new_with_label(last_folder);
    
    gtk_widget_set_tooltip_text (button_outfolder, bimp_output_folder);
    gtk_widget_set_size_request(button_outfolder, 175, 30);
    
    button_samefolder = gtk_button_new();
    GtkWidget* samefolder_icon = gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(button_samefolder), samefolder_icon);
    gtk_widget_set_tooltip_text (button_samefolder, _("Use the selected file's location as the output"));
    gtk_widget_set_size_request(button_samefolder, 30, 30);
    
    bimp_opt_alertoverwrite = BIMP_ASK_OVERWRITE; 
    //check_alertoverwrite = gtk_check_button_new_with_label(_("Alert when overwriting existing files"));
    //gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_alertoverwrite), TRUE); 
    
    bimp_opt_keepfolderhierarchy = FALSE;
    check_keepfolderhierarchy = gtk_check_button_new_with_label(_("Keep folder hierarchy"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_keepfolderhierarchy), bimp_opt_keepfolderhierarchy);
    
    /* TODO? */
    bimp_opt_deleteondone = FALSE;
    check_deleteondone = gtk_check_button_new_with_label(_("Delete original file when done"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_deleteondone), bimp_opt_deleteondone);
    
    bimp_opt_keepdates = FALSE;
    check_keepdates = gtk_check_button_new_with_label(_("Keep the modification dates"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_keepdates), bimp_opt_keepdates);
    
    button_preview = gtk_button_new();
    gtk_widget_set_size_request(button_preview, FILE_PREVIEW_W, FILE_PREVIEW_H);
    gtk_button_set_image_position (GTK_BUTTON(button_preview), GTK_POS_TOP);
    gtk_button_set_label(GTK_BUTTON(button_preview), _("Click for preview"));
    
    /* All together */
    gtk_box_pack_start(GTK_BOX(hbox_buttons), button_add, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), button_remove, FALSE, FALSE, 0);
    
    gtk_container_add (GTK_CONTAINER(scroll_input), treeview_files);
        
    gtk_box_pack_start(GTK_BOX(vbox_input), scroll_input, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_input), hbox_buttons, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox_outfolder), label_chooser, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_outfolder), button_outfolder, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_outfolder), button_samefolder, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox_useroptions), hbox_outfolder, FALSE, FALSE, 0);
    //gtk_box_pack_start(GTK_BOX(vbox_useroptions), check_alertoverwrite, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_useroptions), check_keepfolderhierarchy, FALSE, FALSE, 0);
    // TODO: delete on done? gtk_box_pack_start(GTK_BOX(vbox_useroptions), check_deleteondone, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_useroptions), check_keepdates, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_useroptions), button_preview, FALSE, FALSE, 2);
    
    gtk_box_pack_start(GTK_BOX(hbox), vbox_input, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox_useroptions, FALSE, FALSE, 10);
    
    gtk_container_add(GTK_CONTAINER(panel), hbox);

    g_signal_connect(G_OBJECT(button_add), "clicked", G_CALLBACK(open_addfiles_popupmenu), NULL);
    g_signal_connect(G_OBJECT(button_remove), "clicked", G_CALLBACK(open_removefiles_popupmenu), NULL);
    g_signal_connect(G_OBJECT(button_outfolder), "clicked", G_CALLBACK(open_outputfolder_chooser), NULL);
    g_signal_connect(G_OBJECT(button_samefolder), "clicked", G_CALLBACK(set_source_output_folder), NULL);
    g_signal_connect(G_OBJECT(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_files))), "changed", G_CALLBACK(select_filename), NULL);
    g_signal_connect(G_OBJECT(button_preview), "clicked", G_CALLBACK(show_preview), NULL);
    
    init_fileview();
    bimp_refresh_fileview();
    
    return panel;
}

/* following: functions that modify the file list widget (addfile/addfolder/remove/removeall)  */

static void add_input_file(char* filename) 
{
    if (g_slist_find_custom(bimp_input_filenames, filename, (GCompareFunc)strcmp) == NULL) {
        bimp_input_filenames = g_slist_append(bimp_input_filenames, filename);
        bimp_refresh_fileview();
    }
}

/* Recursive function to add all files from the hierarchy if desired */
static void add_input_folder_r(char* folder, gboolean with_subdirs) 
{
    GDir *dp;
    const gchar* entry;
    dp = g_dir_open (folder, 0, NULL);
    
    if (dp != NULL) {
        while (entry = g_dir_read_name (dp)) {            
            
            char* filename = g_strconcat(folder, FILE_SEPARATOR_STR, entry, NULL);
            char* file_extension = g_strdup(strrchr(filename, '.'));
            GError *error;
            GFileInfo *file_info = g_file_query_info (g_file_new_for_path(filename), "standard::*", 0, NULL, &error);
            
            /* Folder processing */
            if (g_file_info_get_file_type(file_info) == G_FILE_TYPE_DIRECTORY){
                if (g_strcmp0(entry, ".") == 0 || g_strcmp0(entry, "..") == 0)
                    continue;
                if (with_subdirs) 
                    add_input_folder_r(filename, with_subdirs);
                continue;
            }
                        
            if ((
                g_ascii_strcasecmp(file_extension, ".bmp") == 0 ||
                g_ascii_strcasecmp(file_extension, ".jpeg") == 0 ||
                g_ascii_strcasecmp(file_extension, ".jpg") == 0 ||
                g_ascii_strcasecmp(file_extension, ".jpe") == 0 ||
                g_ascii_strcasecmp(file_extension, ".gif") == 0 ||
                g_ascii_strcasecmp(file_extension, ".png") == 0 ||
                g_ascii_strcasecmp(file_extension, ".tif") == 0 ||
                g_ascii_strcasecmp(file_extension, ".tiff") == 0 ||
                g_ascii_strcasecmp(file_extension, ".svg") == 0 ||
                g_ascii_strcasecmp(file_extension, ".xcf") == 0) && 
                g_slist_find_custom(bimp_input_filenames, filename, (GCompareFunc)strcmp) == NULL)
            {
                bimp_input_filenames = g_slist_append(bimp_input_filenames, filename);
            }
        }
        g_dir_close (dp);
    }
    else {
        bimp_show_error_dialog(g_strdup_printf(_("Couldn't read into \"%s\" directory."), folder), bimp_window_main);
    }
}

static void add_opened_files() 
{
    gint num_images = 0;
    int* image_ids = gimp_image_list (&num_images);
    int i;
    gboolean missing = FALSE;
    for (i = 0; i < num_images; i++) {
        gchar* uri = gimp_image_get_uri(image_ids[i]);
        if (uri != NULL) {
            gchar* path = g_filename_from_uri(uri, NULL, NULL);
            if (path != NULL) add_input_file (path);
        }
        else missing = TRUE;
    }
    
    if (missing) {
        bimp_show_error_dialog(g_strdup_printf(_("Some images were not imported because they have not been saved on filesystem yet.")), bimp_window_main);
    }
    
    g_free(image_ids);
}

static void add_input_folder(char* folder, gpointer with_subdirs) 
{
    add_input_folder_r(folder, (gboolean)GPOINTER_TO_INT(with_subdirs));
    bimp_refresh_fileview();
}

/* returns the list of currently selected filenames (NULL of none) */
static GSList* get_treeview_selection() 
{
    GtkTreeModel *model;
    GList *selected_rows = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_files)), &model);
    GList *i = NULL;
    GSList *out = NULL;
    
    if (selected_rows != NULL) {
        for (i = selected_rows; i != NULL; i = g_list_next(i) ) {
            GtkTreeIter iter;
            char* selected_i;
            if (gtk_tree_model_get_iter(model, &iter, (GtkTreePath*)i->data) == TRUE) {
                gtk_tree_model_get(model, &iter, LIST_ITEM, &selected_i, -1);
                out = g_slist_append(out, selected_i);
            }
        }
        
        g_list_foreach (selected_rows, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (selected_rows);
    }
    
    return out;
}

static void remove_input_file(GtkWidget *widget, gpointer data) 
{
    GSList *selection = get_treeview_selection();
    GSList *i;
    
    if (selection != NULL) {
        for (i = selection; i != NULL; i = g_slist_next(i) ) {
            bimp_input_filenames = g_slist_delete_link(bimp_input_filenames, g_slist_find_custom(bimp_input_filenames, (char*)(i->data), (GCompareFunc)strcmp));
        }
        
        bimp_refresh_fileview();
        update_selection(NULL); /* clear the preview widget */
    }
}

static void remove_all_input_files(GtkWidget *widget, gpointer data) 
{
    g_slist_free(bimp_input_filenames);
    bimp_input_filenames = NULL;
    bimp_refresh_fileview();
    update_selection(NULL);
}

/* called when the user clicks on a filename row to update the preview widget */
static void select_filename (GtkTreeView *tree_view, gpointer data) 
{
    GSList *selection = get_treeview_selection();
    
    if (selection != NULL && g_slist_length(selection) == 1) {
        update_selection((gchar*)(selection->data));
    }
    else {
        update_selection(NULL);
    }
}

/* updates the GUI according to the current selected filename */
static void update_selection (gchar* filename) 
{    
    g_free(selected_source_folder);
    if (filename != NULL) {
        // update preview
        GdkPixbuf *pixbuf_prev = gdk_pixbuf_new_from_file_at_scale(filename, FILE_PREVIEW_W - 20, FILE_PREVIEW_H - 30, TRUE, NULL);
        gtk_button_set_image(GTK_BUTTON(button_preview), gtk_image_new_from_pixbuf (pixbuf_prev));
        gtk_widget_show(button_preview);
        
        // update current selection
        selected_source_folder = g_path_get_dirname(filename);
    } else {
        // invalidate
        gtk_button_set_image(GTK_BUTTON(button_preview), NULL);
        gtk_widget_hide(button_preview);
        selected_source_folder = NULL;
    }
    
    gtk_widget_set_sensitive(button_samefolder, (selected_source_folder != NULL));
}

/* opens a dialog containing two panels: the original selected image on the left and the result of the selected manipulations on the right */
static void show_preview (GtkTreeView *tree_view, gpointer data) 
{
    if (g_slist_length(bimp_selected_manipulations) == 0) {
        bimp_show_error_dialog(_("Can't show a preview because the manipulations set is empty"), bimp_window_main);
    } else {
        GtkWidget *dialog_preview;
        GtkWidget *vbox, *hbox;
        GtkWidget *label_descr;
        GdkPixbuf *pixbuf_orig, *pixbuf_final; 
        GtkWidget *image_orig, *image_final;
        GtkWidget *image_forward;
        
        GSList* selection = get_treeview_selection();
        if (selection == NULL && g_slist_length(selection) != 1) return;
        
        char* selected_str = g_slist_nth_data(selection, 0);
        
        image_output imageout_orig = (image_output)g_malloc(sizeof(struct imageout_str));
        image_output imageout_final = (image_output)g_malloc(sizeof(struct imageout_str));
        
        imageout_orig->image_id = gimp_file_load(GIMP_RUN_NONINTERACTIVE, (gchar*)selected_str, (gchar*)selected_str);
        int imageout_orig_drawable = gimp_image_merge_visible_layers(imageout_orig->image_id, GIMP_CLIP_TO_IMAGE); 
        
        bimp_init_batch();
        bimp_apply_drawable_manipulations(imageout_final, (gchar*)selected_str, (gchar*)selected_str);
        int imageout_final_drawable = gimp_image_merge_visible_layers(imageout_final->image_id, GIMP_CLIP_TO_IMAGE); 
        
        pixbuf_orig = gimp_drawable_get_thumbnail(imageout_orig_drawable, PREVIEW_IMG_W, PREVIEW_IMG_H, GIMP_PIXBUF_KEEP_ALPHA);
        pixbuf_final = gimp_drawable_get_thumbnail(imageout_final_drawable, PREVIEW_IMG_W, PREVIEW_IMG_H, GIMP_PIXBUF_KEEP_ALPHA);
        
        dialog_preview = gtk_dialog_new_with_buttons (
            _("Preview"),
            GTK_WINDOW(bimp_window_main),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_STOCK_CLOSE,
            GTK_RESPONSE_CLOSE, NULL
        );
        gtk_widget_set_size_request (dialog_preview, PREVIEW_WINDOW_W, PREVIEW_WINDOW_H);
        gtk_window_set_resizable (GTK_WINDOW(dialog_preview), FALSE);
        gtk_window_set_position(GTK_WINDOW(dialog_preview), GTK_WIN_POS_CENTER);
        gtk_container_set_border_width(GTK_CONTAINER(dialog_preview), 5);
        
        vbox = gtk_vbox_new(FALSE, 10);
        label_descr = gtk_label_new(_("This is how the selected image will look like after the batch process"));
        
        hbox = gtk_hbox_new(FALSE, 10);
        image_orig = gtk_image_new_from_pixbuf(pixbuf_orig);
        image_forward = gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_BUTTON);
        image_final = gtk_image_new_from_pixbuf(pixbuf_final);
        
        gtk_box_pack_start(GTK_BOX(hbox), image_orig, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), image_forward, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), image_final, FALSE, FALSE, 0);
        
        gtk_box_pack_start(GTK_BOX(vbox), label_descr, FALSE, FALSE, 7);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
        
        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog_preview)->vbox), vbox);
        gtk_widget_show_all(dialog_preview);
        
        if (gtk_dialog_run (GTK_DIALOG(dialog_preview)) == GTK_RESPONSE_CLOSE) {
            gtk_widget_destroy (dialog_preview);
            gimp_image_delete(imageout_orig->image_id);
            gimp_image_delete(imageout_final->image_id);
            g_free(imageout_orig);
            g_free(imageout_final);
        }
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
void bimp_refresh_fileview() 
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
    if (g_slist_length(bimp_input_filenames) > 0) {
        iter = bimp_input_filenames;
        for (; iter; iter = iter->next) {
            add_to_fileview(iter->data);
        }
    }
}

static void open_file_chooser(GtkWidget *widget, gpointer data) 
{
    GSList *selection;
    
    GtkFileFilter *filter_all, *supported[9];

    GtkWidget* file_chooser = gtk_file_chooser_dialog_new(
        _("Select images"), 
        NULL, 
        GTK_FILE_CHOOSER_ACTION_OPEN, 
        GTK_STOCK_CANCEL, GTK_RESPONSE_CLOSE, 
        GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT, NULL
    );
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser), TRUE);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser), last_input_location);
    
    filter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_all,_("All supported types"));
    
    supported[0] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[0], "Bitmap (*.bmp)");
    gtk_file_filter_add_pattern (supported[0], "*.[bB][mM][pP]");
    gtk_file_filter_add_pattern (filter_all, "*.[bB][mM][pP]");
    
    supported[1] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[1], "JPEG (*.jpg, *.jpeg, *jpe)");
    gtk_file_filter_add_pattern (supported[1], "*.[jJ][pP][gG]");
    gtk_file_filter_add_pattern (supported[1], "*.[jJ][pP][eE][gG]");
    gtk_file_filter_add_pattern (supported[1], "*.[jJ][pP][eE]");
    gtk_file_filter_add_pattern (filter_all, "*.[jJ][pP][gG]");
    gtk_file_filter_add_pattern (filter_all, "*.[jJ][pP][eE][gG]");
    gtk_file_filter_add_pattern (filter_all, "*.[jJ][pP][eE]");

    supported[2] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[2], "GIF (*.gif)");
    gtk_file_filter_add_pattern (supported[2], "*.[gG][iI][fF]");
    gtk_file_filter_add_pattern (filter_all, "*.[gG][iI][fF]");
    
    supported[3] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[3], "PNG (*.png)");
    gtk_file_filter_add_pattern (supported[3], "*.[pP][nN][gG]");
    gtk_file_filter_add_pattern (filter_all, "*.[pP][nN][gG]");
    
    supported[4] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[4], "Icon (*.ico)");
    gtk_file_filter_add_pattern (supported[4], "*.[iI][cC][oO]");
    gtk_file_filter_add_pattern (filter_all, "*.[iI][cC][oO]");
    
    supported[5] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[5], "Scalable Vector Graphics (*.svg)");
    gtk_file_filter_add_pattern (supported[5], "*.[sS][vV][gG]");
    gtk_file_filter_add_pattern (filter_all, "*.[sS][vV][gG]");

    supported[6] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[6], "TIFF (*tif, *.tiff)");
    gtk_file_filter_add_pattern (supported[6], "*.[tT][iI][fF][fF]");
    gtk_file_filter_add_pattern (supported[6], "*.[tT][iI][fF]");
    gtk_file_filter_add_pattern (filter_all, "*.[tT][iI][fF][fF]");
    gtk_file_filter_add_pattern (filter_all, "*.[tT][iI][fF]");
    
    supported[7] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[7], "Targa (*.tga)");
    gtk_file_filter_add_pattern (supported[7], "*.[tT][gG][aA]");
    gtk_file_filter_add_pattern (filter_all, "*.[tT][gG][aA]");

    supported[8] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[8], "GIMP XCF (*.xcf)");
    gtk_file_filter_add_pattern (supported[8], "*.[xX][cC][fF]");
    gtk_file_filter_add_pattern (filter_all, "*.[xX][cC][fF]");
        
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), filter_all);
    size_t i;
    for(i = 0; i < 9; i++) {
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), supported[i]);
    }
    
    /* show dialog */
    if (gtk_dialog_run (GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT) {
        selection = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(file_chooser));
        
        g_free(last_input_location);
        last_input_location = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(file_chooser));
        
        g_slist_foreach(selection, (GFunc)add_input_file, NULL);
    }
    
    gtk_widget_destroy (file_chooser);
}

static void open_folder_chooser(GtkWidget *widget, gpointer data) 
{
    GSList *selection;
    gboolean include_subdirs;

    GtkWidget* folder_chooser = gtk_file_chooser_dialog_new(
        _("Select folders containing images"), 
        NULL, 
        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, 
        GTK_STOCK_CANCEL, GTK_RESPONSE_CLOSE, 
        GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT, NULL
    );
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(folder_chooser), TRUE);

    /* Add checkbox to select the depth of file search */
    GtkWidget* with_subdirs = gtk_check_button_new_with_label(_("Add files from the whole hierarchy"));
    gtk_widget_show (with_subdirs);
    gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER(folder_chooser), GTK_WIDGET(with_subdirs));
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(folder_chooser), last_input_location);
    
    /* show dialog */
    if (gtk_dialog_run (GTK_DIALOG(folder_chooser)) == GTK_RESPONSE_ACCEPT) {
        selection = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(folder_chooser));
        
        g_free(last_input_location);
        last_input_location = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(folder_chooser));
        
        include_subdirs = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(with_subdirs));
        g_slist_foreach(selection, (GFunc)add_input_folder, GINT_TO_POINTER(include_subdirs));
    }
    
    gtk_widget_destroy (folder_chooser);
}

static void open_outputfolder_chooser(GtkWidget *widget, gpointer data) 
{
    GtkWidget* chooser = gtk_file_chooser_dialog_new(
        _("Select output folder"), 
        NULL, 
        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, 
        GTK_STOCK_CANCEL, GTK_RESPONSE_CLOSE, 
        GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL
    );
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(chooser), FALSE);
    if (selected_source_folder != NULL) gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), selected_source_folder);
    
    if (gtk_dialog_run (GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
        bimp_output_folder = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(chooser))->data;
        
        char* last_folder = g_strrstr(bimp_output_folder, FILE_SEPARATOR_STR) + 1;
        if (last_folder == NULL || strlen(last_folder) == 0) last_folder = bimp_output_folder;
        gtk_button_set_label(GTK_BUTTON(button_outfolder), last_folder);
        
        gtk_widget_set_tooltip_text(button_outfolder, bimp_output_folder);
    }
    
    gtk_widget_destroy (chooser);
}

static void set_source_output_folder(GtkWidget *widget, gpointer data) 
{
    if (selected_source_folder != NULL) {
        char* last_folder = g_strrstr(selected_source_folder, FILE_SEPARATOR_STR) + 1;
        if (last_folder == NULL || strlen(last_folder) == 0) last_folder = selected_source_folder;
        
        gtk_button_set_label(GTK_BUTTON(button_outfolder), last_folder);
        gtk_widget_set_tooltip_text(button_outfolder, selected_source_folder);
        bimp_output_folder = g_strdup(selected_source_folder);
    }
}

/* initializes the two menus that appears when the user clicks on the "add new" button 
 * or on one of the already added manipulations */
static void popmenus_init() 
{
    GtkWidget *menuitem;
    
    /* Menu to add a manipulation */
    popmenu_add = gtk_menu_new();
    
    int man_id;
    for (man_id = 0; man_id < MANIP_END; man_id++)
    {
        menuitem = gtk_menu_item_new_with_label(bimp_manip_get_string(man_id));
        g_signal_connect(menuitem, "activate", G_CALLBACK(add_manipulation_from_id), GINT_TO_POINTER(man_id));
        gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_add), menuitem);
    }
    
    
    /* last items to save and load a manipulations set */
    
    menuitem = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_add), menuitem);
    
    menuitem = gtk_menu_item_new_with_label(_("Save this set..."));
    g_signal_connect(menuitem, "activate", G_CALLBACK(save_set), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_add), menuitem);
    
    menuitem = gtk_menu_item_new_with_label(_("Load set..."));
    g_signal_connect(menuitem, "activate", G_CALLBACK(load_set), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_add), menuitem);
    
    /* menu to edit a manipulation */
    popmenu_edit = gtk_menu_new();
    
    menuitem = gtk_menu_item_new_with_label("<step name>"); /* first element shows only the step name */
    gtk_widget_set_sensitive(menuitem, FALSE); /* and it's non-selectable */
    
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_edit), menuitem);
    menuitem = gtk_menu_item_new_with_label(_("Edit properties..."));
    g_signal_connect(menuitem, "activate", G_CALLBACK(edit_clicked_manipulation), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_edit), menuitem);
    menuitem = gtk_menu_item_new_with_label(_("Remove this manipulation"));
    g_signal_connect(menuitem, "activate", G_CALLBACK(remove_clicked_manipulation), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_edit), menuitem);
    
    /* menu to add files to the list in various ways */
    popmenu_addfiles = gtk_menu_new();
    
    menuitem = gtk_menu_item_new_with_label(_("Add single images..."));
    g_signal_connect(menuitem, "activate", G_CALLBACK(open_file_chooser), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_addfiles), menuitem);
    menuitem = gtk_menu_item_new_with_label(_("Add folders..."));
    g_signal_connect(menuitem, "activate", G_CALLBACK(open_folder_chooser), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_addfiles), menuitem);
    menuitem = gtk_menu_item_new_with_label(_("Add all opened images"));
    g_signal_connect(menuitem, "activate", G_CALLBACK(add_opened_files), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_addfiles), menuitem);
    
    /* menu to remove files to the list */
    popmenu_removefiles = gtk_menu_new();
    
    menuitem = gtk_menu_item_new_with_label(_("Remove selected"));
    g_signal_connect(menuitem, "activate", G_CALLBACK(remove_input_file), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_removefiles), menuitem);
    menuitem = gtk_menu_item_new_with_label(_("Remove all"));
    g_signal_connect(menuitem, "activate", G_CALLBACK(remove_all_input_files), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_removefiles), menuitem);
    
    gtk_widget_show_all(popmenu_add);
    gtk_widget_show_all(popmenu_edit);
    gtk_widget_show_all(popmenu_addfiles);
    gtk_widget_show_all(popmenu_removefiles);
}

static void open_manipulation_popupmenu(GtkWidget *widget, gpointer data)
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
            item_label = bimp_manip_get_string(clicked_man->type);
        }
        gtk_menu_item_set_label(g_list_first(gtk_container_get_children(GTK_CONTAINER(popmenu_edit)))->data, item_label);
        gtk_menu_popup(GTK_MENU(popmenu_edit), NULL, NULL, NULL, NULL, 0, 0);
    }
}

static void open_addfiles_popupmenu(GtkWidget *widget, gpointer data)
{
    gtk_menu_popup(GTK_MENU(popmenu_addfiles), NULL, NULL, NULL, NULL, 0, 0);
}

static void open_removefiles_popupmenu(GtkWidget *widget, gpointer data)
{
    gtk_menu_popup(GTK_MENU(popmenu_removefiles), NULL, NULL, NULL, NULL, 0, 0);
}

static void add_manipulation_from_id(GtkMenuItem *menuitem, gpointer id) 
{    
    int man_id = GPOINTER_TO_INT(id);
    manipulation newman = bimp_append_manipulation((manipulation_type)man_id);
    if (newman == NULL) {
        bimp_show_error_dialog(_("Can't add another manipulation of this kind. Only one is permitted!"), bimp_window_main);
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
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(open_manipulation_popupmenu), NULL);
    
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
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(open_manipulation_popupmenu), man);
}

static void save_set(GtkMenuItem *menuitem, gpointer user_data)
{
    if (g_slist_length(bimp_selected_manipulations) == 0) {
        bimp_show_error_dialog(_("The manipulations set is empty!"), bimp_window_main);
    } else {
        gchar* output_file;
        char* extension = ".bimp";
        
        GtkWidget* file_saver = gtk_file_chooser_dialog_new(
            _("Save this set..."), 
            NULL, 
            GTK_FILE_CHOOSER_ACTION_SAVE, 
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
            GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL
        );
        
        GtkFileFilter *filter_bimp = gtk_file_filter_new();
        gtk_file_filter_set_name(filter_bimp,"BIMP manipulations set (*.bimp)");
        gtk_file_filter_add_pattern (filter_bimp, "*.bimp");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_saver), filter_bimp);
        
        if (gtk_dialog_run (GTK_DIALOG(file_saver)) == GTK_RESPONSE_ACCEPT) {
            
            output_file = g_strdup(g_slist_nth (gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(file_saver)), 0)->data);
            if (!g_str_has_suffix(output_file, extension)) {
                output_file = g_strconcat(output_file, extension, NULL);
            }
            gtk_widget_destroy (file_saver);
            
            if (!bimp_serialize_to_file(output_file)) {
                bimp_show_error_dialog(_("An error occured when importing a saved batch file :("), bimp_window_main);
            }
            
            return;
        }
        
        gtk_widget_destroy (file_saver);
        return;
    }
}

static void load_set(GtkMenuItem *menuitem, gpointer user_data)
{
    gboolean can_continue = TRUE;
    
    if (g_slist_length(bimp_selected_manipulations) > 0) {
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new(
            GTK_WINDOW(bimp_window_main),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_QUESTION,
            GTK_BUTTONS_YES_NO,
            _("This will overwrite current manipulations set. Continue?")
        );
        gtk_window_set_title(GTK_WINDOW(dialog), "Continue?");
        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
         
        can_continue = (result == GTK_RESPONSE_YES);
    }
    
    if (can_continue) {
        gchar* input_file;
        GtkWidget* file_loader = gtk_file_chooser_dialog_new(
            _("Load set..."), 
            NULL, 
            GTK_FILE_CHOOSER_ACTION_OPEN, 
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
            GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL
        );
        
        GtkFileFilter *filter_bimp = gtk_file_filter_new();
        gtk_file_filter_set_name(filter_bimp, "BIMP manipulations set (*.bimp)");
        gtk_file_filter_add_pattern (filter_bimp, "*.bimp");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_loader), filter_bimp);
        
        if (gtk_dialog_run (GTK_DIALOG(file_loader)) == GTK_RESPONSE_ACCEPT) {
            input_file = g_strdup(g_slist_nth (gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(file_loader)), 0)->data);
            gtk_widget_destroy (file_loader);
            
            if (!bimp_deserialize_from_file(input_file)) {
                bimp_show_error_dialog(_("An error occured when importing a saved batch file :("), bimp_window_main);
            }
            else {
                bimp_refresh_sequence_panel();
            }
            return;
        }
        
        gtk_widget_destroy (file_loader);
        return;
    }
}

static void open_about() 
{
    const gchar *auth[] = { 
        "Alessandro Francesconi <alessandrofrancesconi@live.it>", 
        "Thomas Mevel <thomas.prog@mevtho.com>", 
        "Walt9Z <https://github.com/Walt9Z>",
        "KHam0425 <https://github.com/KHam0425>",
        NULL };
    const gchar *license = 
        "This program is free software; you can redistribute it and/or modify "
        "it under the terms of the GNU General Public License as published by "
        "the Free Software Foundation; either version 2 of the License, or "
        "(at your option) any later version. \n\n"
        "This program is distributed in the hope that it will be useful, "
        "but WITHOUT ANY WARRANTY; without even the implied warranty of "
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
        "GNU General Public License for more details.\n\n"
        "You should have received a copy of the GNU General Public License "
        "along with this program; if not, write to the Free Software "
        "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, "
        "MA 02110-1301, USA. ";

    gtk_show_about_dialog( 
        GTK_WINDOW(bimp_window_main),
        "program-name", PLUG_IN_FULLNAME,
        "version", PLUG_IN_VERSION,
        "comments", _("Applies GIMP manipulations on groups of images"),
        "logo", gdk_pixbuf_from_pixdata(&pixdata_bimpicon, FALSE, NULL),
        "copyright", PLUG_IN_COPYRIGHT,
        "license", license,
        "wrap-license", TRUE,
        "website", PLUG_IN_WEBSITE,
        "authors", auth,
        "translator-credits", _("translator-name <translator-email>"),
        NULL 
    );
}

/* shows an error dialog with a custom message */
void bimp_show_error_dialog(char* message, GtkWidget* parent) 
{
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
    if (fraction > 1.0) fraction = 1.0;
    
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar_visible), fraction);
    if (text != NULL) {
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar_visible), text);
    }
}

void bimp_set_busy(gboolean busy) {
    GList *actions_children, *tmp_child;
    struct _ResponseData { gint response_id; };
    
    bimp_is_busy = busy;
    
    gtk_dialog_set_response_sensitive (GTK_DIALOG(bimp_window_main), GTK_RESPONSE_CLOSE, !busy);
    gtk_dialog_set_response_sensitive (GTK_DIALOG(bimp_window_main), GTK_RESPONSE_HELP, !busy);
    
    /* procedure that hides and shows some widgets in the dialog's action area. Compatible with GTK+ 2.16 */
    GtkWidget* actions = gtk_dialog_get_action_area (GTK_DIALOG(bimp_window_main));
    actions_children = gtk_container_get_children (GTK_CONTAINER (actions));
    tmp_child = actions_children;
    while (tmp_child != NULL)
    {
        GtkWidget *widget = tmp_child->data;
        struct _ResponseData *rd = g_object_get_data (G_OBJECT (widget), "gtk-dialog-response-data");

        if (rd && rd->response_id == GTK_RESPONSE_APPLY) {
            if (busy) {
                gtk_widget_hide (widget);
            } else {
                gtk_widget_show (widget);
            }
        }
        else if (rd && rd->response_id == GTK_RESPONSE_CANCEL) {
            if (!busy) {
                gtk_widget_hide (widget);
            } else {
                gtk_widget_show (widget);
            }
        }

        tmp_child = g_list_next (tmp_child);
    }
    g_list_free (actions_children);
    
    gtk_widget_set_sensitive(panel_sequence, !busy);
    gtk_widget_set_sensitive(panel_options, !busy);
}