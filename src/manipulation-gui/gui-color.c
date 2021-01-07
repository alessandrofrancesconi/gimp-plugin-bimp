#include <gtk/gtk.h>
#include "gui-color.h"
#include "../bimp-manipulations.h"
#include "../bimp-manipulations-gui.h"
#include "../plugin-intl.h"

static void toggle_curve(GtkToggleButton*, gpointer);

static GtkWidget *scale_bright, *scale_contrast;
static GtkWidget *check_autolevels, *check_grayscale, *check_curve;
static GtkWidget *chooser_curve;
    
GtkWidget* bimp_color_gui_new(color_settings settings)
{
    GtkWidget *gui;
    GtkWidget *label_bright, *label_contrast;
    
    gui = gtk_table_new(6, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(gui), 5);
    gtk_table_set_col_spacings(GTK_TABLE(gui), 5);
    
    label_bright = gtk_label_new(g_strconcat(_("Brightness"), ":", NULL));
    gtk_misc_set_alignment(GTK_MISC(label_bright), 0, 0.5);
    scale_bright = gtk_hscale_new_with_range(-0.5, +0.5, 0.01);
    gtk_misc_set_alignment(GTK_MISC(scale_bright), 0, 0.5);
    gtk_range_set_value(GTK_RANGE(scale_bright), settings->brightness);
    
    label_contrast = gtk_label_new(g_strconcat(_("Contrast"), ":", NULL));
    gtk_misc_set_alignment(GTK_MISC(label_contrast), 0, 0.5);
    scale_contrast = gtk_hscale_new_with_range(-0.5, +0.5, 0.01);
    gtk_misc_set_alignment(GTK_MISC(scale_contrast), 0, 0.5);
    gtk_range_set_value(GTK_RANGE(scale_contrast), settings->contrast);
    
    check_grayscale = gtk_check_button_new_with_label(_("Convert to grayscale"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_grayscale), settings->grayscale);
    check_autolevels = gtk_check_button_new_with_label(_("Automatic color levels correction"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_autolevels), settings->levels_auto);
    
    check_curve = gtk_check_button_new_with_label(_("Change color curve from settings file:"));
    
    chooser_curve = gtk_file_chooser_button_new(_("Select GIMP Curve file"), GTK_FILE_CHOOSER_ACTION_OPEN);
    if (settings->curve_file != NULL) {
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(chooser_curve), settings->curve_file);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_curve), TRUE);
    }
    else {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_curve), FALSE);
    }
    
    gtk_table_attach(GTK_TABLE(gui), label_bright, 0, 1, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(gui), scale_bright, 1, 2, 0, 1);
    
    gtk_table_attach(GTK_TABLE(gui), label_contrast, 0, 1, 1, 2, GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(gui), scale_contrast, 1, 2, 1, 2);
    
    gtk_table_attach_defaults(GTK_TABLE(gui), check_grayscale, 0, 2, 2, 3);
    gtk_table_attach_defaults(GTK_TABLE(gui), check_autolevels, 0, 2, 3, 4);
    gtk_table_attach_defaults(GTK_TABLE(gui), check_curve, 0, 2, 4, 5);
    gtk_table_attach_defaults(GTK_TABLE(gui), chooser_curve, 0, 2, 5, 6);
    
    toggle_curve(NULL, NULL);
    g_signal_connect(G_OBJECT(check_curve), "toggled", G_CALLBACK(toggle_curve), NULL);
        
    return gui;
}

void toggle_curve(GtkToggleButton *togglebutton, gpointer user_data) 
{
    gtk_widget_set_sensitive(GTK_WIDGET(chooser_curve), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_curve)));
}

void bimp_color_save(color_settings orig_settings)
{
    orig_settings->brightness = gtk_range_get_value(GTK_RANGE(scale_bright));
    orig_settings->contrast = gtk_range_get_value(GTK_RANGE(scale_contrast));
    orig_settings->grayscale = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_grayscale));
    orig_settings->levels_auto = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_autolevels));
    
    char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser_curve));
    if (filename != NULL && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_curve))) {
        orig_settings->curve_file = g_strdup(filename);
        g_free(filename);
    }
    else {
        orig_settings->curve_file = NULL;
    }
}








