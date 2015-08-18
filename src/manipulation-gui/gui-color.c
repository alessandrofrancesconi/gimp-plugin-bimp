#include <gtk/gtk.h>
#include "gui-color.h"
#include "../bimp-manipulations.h"
#include "../bimp-manipulations-gui.h"
#include "../plugin-intl.h"

static void toggle_curve(GtkToggleButton*, gpointer);

GtkWidget *scale_bright, *scale_contrast;
GtkWidget *check_autolevels, *check_grayscale, *check_curve;
GtkWidget *chooser_curve;
    
GtkWidget* bimp_color_gui_new(color_settings settings)
{
    GtkWidget *gui, *hbox_brightness, *hbox_contrast;
    GtkWidget *label_bright, *label_contrast;
    GtkWidget *align_choosercurve;
    
    gui = gtk_vbox_new(FALSE, 5);
    
    hbox_brightness = gtk_hbox_new(FALSE, 5);
    label_bright = gtk_label_new(_("Brightness"));
    gtk_widget_set_size_request (label_bright, LABEL_BC_W, LABEL_BC_H);
    gtk_misc_set_alignment(GTK_MISC(label_bright), 0.5, 0.8);
    scale_bright = gtk_hscale_new_with_range(-127, 127, 1);
    gtk_range_set_value(GTK_RANGE(scale_bright), settings->brightness);
    gtk_widget_set_size_request (scale_bright, SCALE_BC_W, SCALE_BC_H);
    
    hbox_contrast = gtk_hbox_new(FALSE, 5);
    label_contrast = gtk_label_new(_("Contrast"));
    gtk_widget_set_size_request (label_contrast, LABEL_BC_W, LABEL_BC_H);
    gtk_misc_set_alignment(GTK_MISC(label_contrast), 0.5, 0.8);
    scale_contrast = gtk_hscale_new_with_range(-127, 127, 1);
    gtk_range_set_value(GTK_RANGE(scale_contrast), settings->contrast);
    gtk_widget_set_size_request (scale_contrast, SCALE_BC_W, SCALE_BC_H);
    
    check_grayscale = gtk_check_button_new_with_label(_("Convert to grayscale"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_grayscale), settings->grayscale);
    check_autolevels = gtk_check_button_new_with_label(_("Automatic color levels correction"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_autolevels), settings->levels_auto);
    
    check_curve = gtk_check_button_new_with_label(_("Change color curve from settings file:"));
    
    align_choosercurve = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(align_choosercurve), 0, 5, 20, 0);
    
    chooser_curve = gtk_file_chooser_button_new(_("Select GIMP Curve file"), GTK_FILE_CHOOSER_ACTION_OPEN);
    if (settings->curve_file != NULL) {
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(chooser_curve), settings->curve_file);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_curve), TRUE);
    }
    else {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_curve), FALSE);
    }
    gtk_widget_set_size_request (chooser_curve, INPUT_W, INPUT_H);
    gtk_container_add(GTK_CONTAINER(align_choosercurve), chooser_curve);
    
    gtk_box_pack_start(GTK_BOX(hbox_brightness), label_bright, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_brightness), scale_bright, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox_contrast), label_contrast, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_contrast), scale_contrast, FALSE, FALSE, 0);
        
    gtk_box_pack_start(GTK_BOX(gui), hbox_brightness, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), hbox_contrast, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), check_grayscale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), check_autolevels, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), check_curve, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), align_choosercurve, FALSE, FALSE, 0);
    
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








