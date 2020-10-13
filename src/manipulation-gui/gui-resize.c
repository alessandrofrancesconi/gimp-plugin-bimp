#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <libgimpwidgets/gimpwidgets.h>
#include "gui-resize.h"
#include "../bimp-manipulations.h"
#include "../plugin-intl.h"

static void aspect_ratio_changed(GtkComboBox*, gpointer);
static void units_changed(GtkComboBox*, gpointer);
static void toggle_resolution(GtkToggleButton*, gpointer);
static void update_dimensions();


static GtkWidget *table_res;
static GtkWidget *check_resolution;
static GtkWidget *combo_unitW, *combo_unitH, *combo_aspectratio, *combo_quality;
static GtkWidget *chooser_paddingcolor;
static GtkWidget *spin_width, *spin_height, *spin_resX, *spin_resY;

static resize_mode previous_resize_w, previous_resize_h;
static gdouble last_percent_w_value;
static gdouble last_percent_h_value;
static gint last_pixel_w_value;
static gint last_pixel_h_value;

GtkWidget* bimp_resize_gui_new(resize_settings settings)
{
    GtkWidget *gui, *table_dimensions, *hbox_aspectratio, *hbox_pad, *hbox_quality;
    GtkWidget *align_res;
    GtkWidget *hsep;
    GtkWidget *label_width, *label_height, *label_aspectratio, *label_pad, *label_quality;
    GtkWidget *label_resX, *label_resY, *label_dpi1, *label_dpi2;
    
    previous_resize_w = RESIZE_END;
    previous_resize_h = RESIZE_END;
    last_percent_w_value = settings->new_w_pc;
    last_percent_h_value = settings->new_h_pc;
    last_pixel_w_value = settings->new_w_px;
    last_pixel_h_value = settings->new_h_px;
    
    gui = gtk_vbox_new(FALSE, 10);
    
    // width/height
    
    table_dimensions = gtk_table_new(2, 3, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table_dimensions), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table_dimensions), 5);
    
    label_width = gtk_label_new(g_strconcat(_("Width"), ":", NULL));
    gtk_misc_set_alignment(GTK_MISC(label_width), 0, .5);
    spin_width = gtk_spin_button_new(NULL, 1, 0);
    combo_unitW = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_unitW), _("%"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_unitW), _("px"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_unitW), _("Disable"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_unitW), settings->resize_mode_width);
    
    label_height = gtk_label_new(g_strconcat(_("Height"), ":", NULL));
    gtk_misc_set_alignment(GTK_MISC(label_height), 0, .5);
    spin_height = gtk_spin_button_new(NULL, 1, 0);
    combo_unitH = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_unitH), _("%"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_unitH), _("px"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_unitH), _("Disable"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_unitH), settings->resize_mode_height);
    
    // aspect ratio
    
    hbox_aspectratio = gtk_hbox_new(FALSE, 5);
    label_aspectratio = gtk_label_new(g_strconcat(_("Aspect ratio"), ":", NULL));
    combo_aspectratio = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_aspectratio), _("Stretch"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_aspectratio), _("Preserve"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_aspectratio), _("Pad"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_aspectratio), settings->stretch_mode);
    
    // padding color
    
    hbox_pad = gtk_hbox_new(FALSE, 5);
    label_pad = gtk_label_new(g_strconcat(_("Padding color"), ":", NULL));
    chooser_paddingcolor = gtk_color_button_new_with_color(&(settings->padding_color));
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(chooser_paddingcolor), TRUE);
    gtk_color_button_set_alpha(GTK_COLOR_BUTTON(chooser_paddingcolor), settings->padding_color_alpha);
    gtk_widget_set_sensitive(GTK_WIDGET(chooser_paddingcolor), settings->stretch_mode == STRETCH_PADDED);
    
    // interpolation
    
    hbox_quality = gtk_hbox_new(FALSE, 5);
    label_quality = gtk_label_new(g_strconcat(_("Interpolation"), ":", NULL));
    combo_quality = gimp_enum_combo_box_new((GType)GIMP_TYPE_INTERPOLATION_TYPE);
    gimp_int_combo_box_set_active((GimpIntComboBox*)combo_quality, settings->interpolation);
    
    // resolution
    
    hsep = gtk_hseparator_new();
    
    table_res = gtk_table_new(2, 3, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table_res), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table_res), 5);
    
    check_resolution = gtk_check_button_new_with_label(_("Change resolution"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_resolution), settings->change_res);
    
    align_res = gtk_alignment_new(0, 0.5, 1, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(align_res), 0, 0, 10, 0);
    label_resX = gtk_label_new(g_strconcat(_("X axis"), ":", NULL));
    gtk_misc_set_alignment(GTK_MISC(label_resX), 0, .5);
    spin_resX = gtk_spin_button_new(NULL, 1, 0);
    gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_resX), GTK_ADJUSTMENT(gtk_adjustment_new (settings->new_res_x, 0.005, 65536.000, 1.000, 1, 0)), 0, 3);
    label_resY = gtk_label_new(g_strconcat(_("Y axis"), ":", NULL));
    gtk_misc_set_alignment(GTK_MISC(label_resY), 0, .5);
    spin_resY = gtk_spin_button_new(NULL, 1, 0);
    gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_resY), GTK_ADJUSTMENT(gtk_adjustment_new (settings->new_res_y, 0.005, 65536.000, 1.000, 1, 0)), 0, 3);
    label_dpi1 = gtk_label_new("dpi");
    gtk_misc_set_alignment(GTK_MISC(label_dpi1), 0, .5);
    label_dpi2 = gtk_label_new("dpi");
    gtk_misc_set_alignment(GTK_MISC(label_dpi2), 0, .5);
    
    // pack everything
    
    gtk_table_attach(GTK_TABLE(table_dimensions), label_width, 0, 1, 0, 1, GTK_FILL, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table_dimensions), spin_width, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table_dimensions), combo_unitW, 2, 3, 0, 1, GTK_FILL, 0, 0, 0);
    
    gtk_table_attach(GTK_TABLE(table_dimensions), label_height, 0, 1, 1, 2, GTK_FILL, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table_dimensions), spin_height, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table_dimensions), combo_unitH, 2, 3, 1, 2, GTK_FILL, 0, 0, 0);
    
    gtk_box_pack_start(GTK_BOX(gui), table_dimensions, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox_aspectratio), label_aspectratio, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_aspectratio), combo_aspectratio, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(gui), hbox_aspectratio, TRUE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox_pad), label_pad, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_pad), chooser_paddingcolor, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(gui), hbox_pad, TRUE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox_quality), label_quality, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_quality), combo_quality, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(gui), hbox_quality, TRUE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(gui), hsep, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), check_resolution, TRUE, FALSE, 0);
    
    gtk_table_attach(GTK_TABLE(table_res), label_resX, 0, 1, 0, 1, GTK_FILL, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table_res), spin_resX, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table_res), label_dpi1, 2, 3, 0, 1, GTK_FILL, 0, 0, 0);
    
    gtk_table_attach(GTK_TABLE(table_res), label_resY, 0, 1, 1, 2, GTK_FILL, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table_res), spin_resY, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table_res), label_dpi2, 2, 3, 1, 2, GTK_FILL, 0, 0, 0);
    
    gtk_container_add(GTK_CONTAINER(align_res), table_res);
    gtk_box_pack_start(GTK_BOX(gui), align_res, TRUE, FALSE, 0);
    
    update_dimensions();
    toggle_resolution(NULL, NULL);
    
    g_signal_connect(G_OBJECT(combo_unitW), "changed", G_CALLBACK(units_changed), NULL);
    g_signal_connect(G_OBJECT(combo_unitH), "changed", G_CALLBACK(units_changed), NULL);
    g_signal_connect(G_OBJECT(combo_aspectratio), "changed", G_CALLBACK(aspect_ratio_changed), NULL);
    g_signal_connect(G_OBJECT(check_resolution), "toggled", G_CALLBACK(toggle_resolution), NULL);
    
    return gui;
}

void aspect_ratio_changed(GtkComboBox *combo, gpointer user_data) {
    update_dimensions();
    
    gtk_widget_set_sensitive(GTK_WIDGET(chooser_paddingcolor), gtk_combo_box_get_active(GTK_COMBO_BOX(combo_aspectratio)) == STRETCH_PADDED);
}

void units_changed(GtkComboBox *combo, gpointer user_data) {
    update_dimensions();
}

void update_dimensions() {
    resize_mode resize_w = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_unitW));
    resize_mode resize_h = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_unitH));
    stretch_mode aspect_ratio = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_aspectratio));
    
    // store current values
    if (resize_w != previous_resize_w) {
        if (previous_resize_w == RESIZE_PERCENT) {
            last_percent_w_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
        }
        else if (previous_resize_w == RESIZE_PIXEL) {
            last_pixel_w_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
        }
        
        // configure width spinbox value and range
        if (resize_w == RESIZE_PERCENT) {
            gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_width), GTK_ADJUSTMENT(gtk_adjustment_new(last_percent_w_value, 1, 40960, 0.01, 1, 0)), 0, 2);
        }
        else if (resize_w == RESIZE_PIXEL) {
            gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_width), GTK_ADJUSTMENT(gtk_adjustment_new(last_pixel_w_value, 1, 262144, 1, 10, 0)), 0, 0);
        }
        else {
            gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_width), GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 0, 0, 0, 0)), 0, 0);
        }
    }
    if (resize_h != previous_resize_h) {
        if (previous_resize_h == RESIZE_PERCENT) {
            last_percent_h_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
        }
        else if (previous_resize_h == RESIZE_PIXEL) {
            last_pixel_h_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
        }
        
        // configure height spinbox value and range
        if (resize_h == RESIZE_PERCENT) {
            gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_height), GTK_ADJUSTMENT(gtk_adjustment_new(last_percent_h_value, 1, 40960, 0.01, 1, 0)), 0, 2);
        }
        else if (resize_h == RESIZE_PIXEL) {
            gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_height), GTK_ADJUSTMENT(gtk_adjustment_new(last_pixel_h_value, 1, 262144, 1, 10, 0)), 0, 0);
        }
        else {
            gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_height), GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 0, 0, 0, 0)), 0, 0);
        }
    }
    
    // disable spinner if unit is disabled
    gtk_widget_set_sensitive(GTK_WIDGET(spin_width), resize_w != RESIZE_DISABLE);
    gtk_widget_set_sensitive(GTK_WIDGET(spin_height), resize_h != RESIZE_DISABLE);
    
    // force x and y axis resolution to be the same if aspect ratio is preserve
    if (aspect_ratio == STRETCH_ASPECT) {
        gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_resY), gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(spin_resX)), 0, 3);
    }
    else {
        gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_resY), GTK_ADJUSTMENT(gtk_adjustment_new(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_resY)), 0.005, 65536.000, 1.000, 1, 0)), 0, 3);
    }
    
    // keep width and height values the same if both are expressed in percent
    if(aspect_ratio == STRETCH_ASPECT && resize_w == RESIZE_PERCENT && resize_h == RESIZE_PERCENT) {
        gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_height), gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(spin_width)), 0, 2);
    }
    
    // set resize modes
    previous_resize_w = resize_w;
    previous_resize_h = resize_h;
}

void toggle_resolution(GtkToggleButton *togglebutton, gpointer user_data) 
{
    gtk_widget_set_sensitive(GTK_WIDGET(table_res), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_resolution)));
}

void bimp_resize_save(resize_settings orig_settings) 
{
    orig_settings->resize_mode_width = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_unitW));
    orig_settings->resize_mode_height = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_unitH));

    if (orig_settings->resize_mode_width == RESIZE_PERCENT) {
        orig_settings->new_w_pc = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
        orig_settings->new_w_px = last_pixel_w_value;
    }
    else if (orig_settings->resize_mode_width == RESIZE_PIXEL) {
        orig_settings->new_w_pc = last_percent_w_value;
        orig_settings->new_w_px = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
    }
    else {
        orig_settings->new_w_pc = last_percent_w_value;
        orig_settings->new_w_px = last_pixel_w_value;
    }
    
    if (orig_settings->resize_mode_height == RESIZE_PERCENT) {
        orig_settings->new_h_pc = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
        orig_settings->new_h_px = last_pixel_h_value;
    }
    else if (orig_settings->resize_mode_height == RESIZE_PIXEL) {
        orig_settings->new_h_pc = last_percent_h_value;
        orig_settings->new_h_px = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
    }
    else {
        orig_settings->new_h_pc = last_percent_h_value;
        orig_settings->new_h_px = last_pixel_h_value;
    }
    
    orig_settings->stretch_mode = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_aspectratio));
    
    gtk_color_button_get_color(GTK_COLOR_BUTTON(chooser_paddingcolor), &(orig_settings->padding_color));
    orig_settings->padding_color_alpha = gtk_color_button_get_alpha(GTK_COLOR_BUTTON(chooser_paddingcolor));
    
    orig_settings->interpolation = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_quality));
    
    orig_settings->change_res = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_resolution));
    orig_settings->new_res_x = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_resX));
    orig_settings->new_res_y = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_resY));
}

