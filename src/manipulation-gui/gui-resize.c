#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <libgimpwidgets/gimpwidgets.h>
#include "gui-resize.h"
#include "../bimp-manipulations.h"
#include "../plugin-intl.h"

static void toggle_units_group(GtkToggleButton*, gpointer);
static void toggle_resolution(GtkToggleButton*, gpointer);


GtkWidget *vbox_px, *hbox_res;
GtkWidget *check_resolution;
GtkWidget *radio_size_percent, *radio_size_px, *radio_stretch_allow, *radio_stretch_aspect, *radio_stretch_padded;
GtkWidget *combo_manualsize, *combo_quality;
GtkWidget *chooser_paddingcolor;
GtkWidget *spin_width, *spin_height, *spin_resX, *spin_resY;
GtkWidget *label_unit;

gdouble last_percent_w_value;
gdouble last_percent_h_value;
gint last_pixel_w_value;
gint last_pixel_h_value;

gboolean previous_was_percent;

GtkWidget* bimp_resize_gui_new(resize_settings settings)
{
    GtkWidget *gui, *vbox_dimensions, *hbox_padding, *vbox_resolution, *hbox_values, *hbox_quality;
    GtkWidget *align_manualsize, *align_values, *align_res;
    GtkWidget *label_width, *label_height, *label_quality, *label_resX, *label_resY, *label_dpi;
    
    last_percent_w_value = settings->new_w_pc;
    last_percent_h_value = settings->new_h_pc;
    last_pixel_w_value = settings->new_w_px;
    last_pixel_h_value = settings->new_h_px;
    
    gui = gtk_hbox_new(FALSE, 10);
    
    // "change dimensions" side
    vbox_dimensions = gtk_vbox_new(FALSE, 5);
    
    radio_size_percent = gtk_radio_button_new_with_label (NULL, _("Set to a percentage of the original"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_size_percent), (settings->resize_mode == RESIZE_PERCENT));
    radio_size_px = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio_size_percent), _("Set exact size in pixel"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_size_px), (settings->resize_mode != RESIZE_PERCENT));
    
    align_manualsize = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(align_manualsize), 0, 5, 20, 0);
    
    combo_manualsize = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_manualsize), _("For both dimensions"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_manualsize), _("Width only"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_manualsize), _("Height only"));
    
    if (settings->resize_mode == RESIZE_PIXEL_WIDTH) gtk_combo_box_set_active(GTK_COMBO_BOX(combo_manualsize), 1);
    else if (settings->resize_mode == RESIZE_PIXEL_HEIGHT) gtk_combo_box_set_active(GTK_COMBO_BOX(combo_manualsize), 2);
    else gtk_combo_box_set_active(GTK_COMBO_BOX(combo_manualsize), 0);
    
    gtk_container_add(GTK_CONTAINER(align_manualsize), combo_manualsize);
    
    GtkWidget* separator1 = gtk_hseparator_new();
    
    radio_stretch_allow = gtk_radio_button_new_with_label (NULL, _("Allow stretching"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_stretch_allow), (settings->stretch_mode == STRETCH_ALLOW));
    radio_stretch_aspect = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio_stretch_allow), _("Preserve aspect ratio"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_stretch_aspect), (settings->stretch_mode == STRETCH_ASPECT));
    radio_stretch_padded = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio_stretch_allow), _("Fill with padding"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_stretch_padded), (settings->stretch_mode == STRETCH_PADDED));
    
    chooser_paddingcolor = gtk_color_button_new_with_color(&(settings->padding_color));
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(chooser_paddingcolor), TRUE);
    gtk_color_button_set_alpha(GTK_COLOR_BUTTON(chooser_paddingcolor), settings->padding_color_alpha);
    
    hbox_padding = gtk_hbox_new(FALSE, 5);
    
    GtkWidget* separator2 = gtk_hseparator_new();
    
    align_values = gtk_alignment_new(0.5, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(align_values), 5, 5, 0, 0);
    hbox_values = gtk_hbox_new(FALSE, 5);
    label_width = gtk_label_new(g_strconcat(_("Width"), ":", NULL));
    spin_width = gtk_spin_button_new(NULL, 1, 0);
    gtk_widget_set_size_request (spin_width, SPIN_SIZE_W, SPIN_SIZE_H);
    label_height = gtk_label_new(g_strconcat(_("Height"), ":", NULL));
    spin_height = gtk_spin_button_new(NULL, 1, 0);
    gtk_widget_set_size_request (spin_height, SPIN_SIZE_W, SPIN_SIZE_H);
    label_unit = gtk_label_new("<unit>");
    gtk_widget_set_size_request (label_unit, 25, 25);
    
    hbox_quality = gtk_hbox_new(FALSE, 5);
    label_quality = gtk_label_new(_("Interpolation quality"));
    combo_quality = gimp_enum_combo_box_new((GType)GIMP_TYPE_INTERPOLATION_TYPE);
    gimp_int_combo_box_set_active((GimpIntComboBox*)combo_quality, settings->interpolation);
    
    GtkWidget* separator3 = gtk_vseparator_new();
    
    // "change resolution" side
    vbox_resolution = gtk_vbox_new(FALSE, 5);
    
    check_resolution = gtk_check_button_new_with_label(_("Change resolution"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_resolution), settings->change_res);
    
    align_res = gtk_alignment_new(0.5, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(align_res), 5, 5, 0, 0);
    hbox_res = gtk_hbox_new(FALSE, 5);
    label_resX = gtk_label_new(g_strconcat(_("X axis"), ":", NULL));
    spin_resX = gtk_spin_button_new(NULL, 1, 0);
    gtk_widget_set_size_request (spin_resX, SPIN_SIZE_W, SPIN_SIZE_H);
    gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_resX), GTK_ADJUSTMENT(gtk_adjustment_new (settings->new_res_x, 0.005, 65536.000, 1.000, 1, 0)), 0, 3);
    label_resY = gtk_label_new(g_strconcat(_("Y axis"), ":", NULL));
    spin_resY = gtk_spin_button_new(NULL, 1, 0);
    gtk_widget_set_size_request (spin_resY, SPIN_SIZE_W, SPIN_SIZE_H);
    gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_resY), GTK_ADJUSTMENT(gtk_adjustment_new (settings->new_res_y, 0.005, 65536.000, 1.000, 1, 0)), 0, 3);
    label_dpi = gtk_label_new("dpi");
    
    // pack everything
    
    gtk_box_pack_start(GTK_BOX(vbox_dimensions), radio_size_percent, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_dimensions), radio_size_px, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_dimensions), align_manualsize, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox_dimensions), separator1, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox_dimensions), radio_stretch_allow, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_dimensions), radio_stretch_aspect, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_padding), radio_stretch_padded, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_padding), chooser_paddingcolor, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_dimensions), hbox_padding, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox_dimensions), separator2, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox_values), label_width, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_values), spin_width, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_values), label_height, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_values), spin_height, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_values), label_unit, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(align_values), hbox_values);
    
    gtk_box_pack_start(GTK_BOX(hbox_quality), label_quality, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_quality), combo_quality, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox_dimensions), align_values, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_dimensions), hbox_quality, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox_resolution), check_resolution, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox_res), label_resX, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_res), spin_resX, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_res), label_resY, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_res), spin_resY, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_res), label_dpi, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(align_res), hbox_res);
    gtk_box_pack_start(GTK_BOX(vbox_resolution), align_res, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(gui), vbox_dimensions, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), separator3, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), vbox_resolution, FALSE, FALSE, 0);
    
    
    previous_was_percent = FALSE;
    toggle_units_group(NULL, NULL);
    toggle_resolution(NULL, NULL);
    
    g_signal_connect(G_OBJECT(radio_size_percent), "toggled", G_CALLBACK(toggle_units_group), NULL);
    g_signal_connect(G_OBJECT(radio_size_px), "toggled", G_CALLBACK(toggle_units_group), NULL);
    g_signal_connect(G_OBJECT(combo_manualsize), "changed", G_CALLBACK(toggle_units_group), NULL);
    g_signal_connect(G_OBJECT(radio_stretch_allow), "toggled", G_CALLBACK(toggle_units_group), NULL);
    g_signal_connect(G_OBJECT(radio_stretch_aspect), "toggled", G_CALLBACK(toggle_units_group), NULL);
    g_signal_connect(G_OBJECT(radio_stretch_padded), "toggled", G_CALLBACK(toggle_units_group), NULL);
    g_signal_connect(G_OBJECT(check_resolution), "toggled", G_CALLBACK(toggle_resolution), NULL);
    
    return gui;
}


void toggle_units_group(GtkToggleButton *togglebutton, gpointer user_data) 
{
    gboolean percent_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_size_percent));
    gboolean aspect_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_stretch_aspect));
    
    int combo_manualsize_sel = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_manualsize));
    gboolean both_active = (combo_manualsize_sel == 0);
    gboolean width_active = (combo_manualsize_sel == 1);
    gboolean height_active = (combo_manualsize_sel == 2);
    
    if (percent_active) {
        // store last pixel values
        if (gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_width)) > 0 && gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_height)) > 0) {
            last_pixel_w_value = gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_width));
            last_pixel_h_value = gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_height));
        }
        
        gtk_widget_set_sensitive(GTK_WIDGET(combo_manualsize), FALSE);
        gtk_label_set_text(GTK_LABEL(label_unit), "%");
        
        // set adjustments
        gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_width), GTK_ADJUSTMENT(gtk_adjustment_new (last_percent_w_value, 1, 40960, 0.01, 1, 0)), 0, 2);
        if (aspect_active) {
            gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_height), gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(spin_width)), 0, 2);
        }
        else {
            gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_height), GTK_ADJUSTMENT(gtk_adjustment_new (last_percent_h_value, 1, 40960, 0.01, 1, 0)), 0, 2);
        }
        
        previous_was_percent = TRUE;
    }
    else {
        if (gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_width)) > 0 && gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_height)) > 0) {
            if (previous_was_percent) {
                last_percent_w_value = gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_width));
                last_percent_h_value = gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_height));
            }
            else {
                last_pixel_w_value = gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_width));
                last_pixel_h_value = gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_height));
            }
        }
        
        gtk_widget_set_sensitive(GTK_WIDGET(combo_manualsize), TRUE);
        gtk_label_set_text(GTK_LABEL(label_unit), "px");
        
        gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_width), GTK_ADJUSTMENT(gtk_adjustment_new (last_pixel_w_value, 1, 262144, 1, 10, 0)), 0, 0);
        gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_height), GTK_ADJUSTMENT(gtk_adjustment_new (last_pixel_h_value, 1, 262144, 1, 10, 0)), 0, 0);
        
        previous_was_percent = FALSE;
    }
    
    // adjustments for resolution spinners
    if (aspect_active) {
        gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_resY), gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(spin_resX)), 0, 3);
    }
    else {
        gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_resY), GTK_ADJUSTMENT(gtk_adjustment_new (gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_resY)), 0.005, 65536.000, 1.000, 1, 0)), 0, 3);
    }
    
    gtk_widget_set_sensitive(GTK_WIDGET(spin_width), (percent_active || both_active || width_active));
    gtk_widget_set_sensitive(GTK_WIDGET(spin_height), (percent_active || both_active || height_active));
    gtk_widget_set_sensitive(GTK_WIDGET(chooser_paddingcolor), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_stretch_padded)));
}

void toggle_resolution(GtkToggleButton *togglebutton, gpointer user_data) 
{
    gtk_widget_set_sensitive(GTK_WIDGET(hbox_res), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_resolution)));
}

void bimp_resize_save(resize_settings orig_settings) 
{    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_size_percent))) {
        orig_settings->resize_mode = RESIZE_PERCENT;
        orig_settings->new_w_pc = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
        orig_settings->new_h_pc = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
        orig_settings->new_w_px = last_pixel_w_value;
        orig_settings->new_h_px = last_pixel_h_value;
    }
    else {
        int combo_manualsize_sel = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_manualsize));        
        if (combo_manualsize_sel == 1) orig_settings->resize_mode = RESIZE_PIXEL_WIDTH;
        else if (combo_manualsize_sel == 2) orig_settings->resize_mode = RESIZE_PIXEL_HEIGHT;
        else orig_settings->resize_mode = RESIZE_PIXEL_BOTH;
        
        orig_settings->new_w_pc = last_percent_w_value;
        orig_settings->new_h_pc = last_percent_h_value;
        orig_settings->new_w_px = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
        orig_settings->new_h_px = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
    }
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_stretch_aspect))) orig_settings->stretch_mode = STRETCH_ASPECT;
    else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_stretch_padded))) orig_settings->stretch_mode = STRETCH_PADDED;
    else orig_settings->stretch_mode = STRETCH_ALLOW;
    
    gtk_color_button_get_color(GTK_COLOR_BUTTON(chooser_paddingcolor), &(orig_settings->padding_color));
    orig_settings->padding_color_alpha = gtk_color_button_get_alpha(GTK_COLOR_BUTTON(chooser_paddingcolor));
    
    int interpolation = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_quality));
    if (interpolation == 1) {
        orig_settings->interpolation = GIMP_INTERPOLATION_LINEAR;
    }
    else if (interpolation == 2) {
        orig_settings->interpolation = GIMP_INTERPOLATION_CUBIC;
    }
    else if (interpolation == 3) {
        orig_settings->interpolation = GIMP_INTERPOLATION_LANCZOS;
    }
    else {
        orig_settings->interpolation = GIMP_INTERPOLATION_NONE;
    }
    
    orig_settings->change_res = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_resolution));
    orig_settings->new_res_x = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_resX));
    orig_settings->new_res_y = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_resY));
}

