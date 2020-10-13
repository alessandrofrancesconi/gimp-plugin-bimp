#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "gui-watermark.h"
#include "../bimp-manipulations.h"
#include "../bimp-manipulations-gui.h"
#include "../bimp-utils.h"
#include "../plugin-intl.h"

static void toggle_group(GtkToggleButton*, gpointer);
static void toggle_image_size(GtkToggleButton*, gpointer);
static const char* watermark_pos_get_string(watermark_position);
static const char* watermark_pos_get_abbreviation(watermark_position);
static void file_filters_add_patterns(GtkFileFilter*, GtkFileFilter*, ...);

static GtkWidget *table_text, *vbox_image, *hbox_image_size;
static GtkWidget *radio_text, *radio_image;
static GtkWidget *entry_text;
static GtkWidget *chooser_font, *chooser_color, *chooser_image;
static GtkWidget *check_image_adaptsize, *spin_image_sizepercent, *spin_edge, *combo_image_sizemode;
static GtkWidget *scale_opacity;
static GtkWidget *position_buttons[9];

GtkWidget* bimp_watermark_gui_new(watermark_settings settings)
{
    GtkWidget *gui, *hbox_mode, *vbox_mode_text, *vbox_mode_image, *vbox_position;
    GtkWidget *hbox_opacity, *hbox_edge;
    GtkWidget *frame_position, *table_position;
    GtkWidget *align_radio_text, *align_radio_image;
    GtkWidget *label_text, *label_font, *label_color, *label_opacity, *label_percent, *label_edge, *label_percentof, *label_px;
    
    gui = gtk_vbox_new(FALSE, 10);
    
    hbox_mode = gtk_hbox_new(FALSE, 15);
    vbox_mode_text = gtk_vbox_new(FALSE, 5);
    GtkWidget* separator1 = gtk_vseparator_new();
    vbox_mode_image = gtk_vbox_new(FALSE, 5);
    
    align_radio_text = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_text), 0, 5, 20, 0);
    radio_text = gtk_radio_button_new_with_label(NULL, _("Text watermark"));
    
    align_radio_text = gtk_alignment_new(0, 0, 1, 1);
    gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_text), 0, 0, 20, 0);
    table_text = gtk_table_new(3, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table_text), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table_text), 5);
    
    label_text = gtk_label_new(g_strconcat(_("Text"), ":", NULL));
    gtk_misc_set_alignment(GTK_MISC(label_text), 0, .5);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_text), settings->mode);
    entry_text = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_text), 50);
    gtk_entry_set_text(GTK_ENTRY(entry_text), settings->text);
    
    label_font = gtk_label_new(g_strconcat(_("Font"), ":", NULL));
    gtk_misc_set_alignment(GTK_MISC(label_font), 0, .5);
    chooser_font = gtk_font_button_new_with_font(pango_font_description_to_string(settings->font));
    
    label_color = gtk_label_new(g_strconcat(_("Color"), ":", NULL));
    gtk_misc_set_alignment(GTK_MISC(label_color), 0, .5);
    chooser_color = gtk_color_button_new_with_color(&(settings->color));
    
    align_radio_image = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_image), 0, 5, 20, 0);
    radio_image = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radio_text), _("Image watermark"));
    
    align_radio_image = gtk_alignment_new(0, 0, 1, 1);
    gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_image), 0, 0, 20, 0);
    vbox_image = gtk_vbox_new(FALSE, 5);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_image), !settings->mode);
    chooser_image = gtk_file_chooser_button_new(_("Select image"), GTK_FILE_CHOOSER_ACTION_OPEN);
    
    check_image_adaptsize = gtk_check_button_new_with_label(g_strconcat(_("Adaptive size"), NULL));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_image_adaptsize), settings->image_sizemode != WM_IMG_NOSIZE);
    hbox_image_size = gtk_hbox_new(FALSE, 5);
    spin_image_sizepercent = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new (settings->image_size_percent, 0.1, 100.0, 0.1, 1, 0)), 1, 1);
    label_percentof = gtk_label_new(g_strconcat(_("% of"), NULL));
    
    combo_image_sizemode = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_image_sizemode), _("Width"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_image_sizemode), _("Height"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_image_sizemode), settings->image_sizemode == WM_IMG_SIZEH ? 1 : 0);
    
    /* set image chooser's filters */
    GtkFileFilter *filter_all, *supported[5];    
    filter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_all, _("All supported types"));
    
    supported[0] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[0], "Bitmap (*.bmp)");
    file_filters_add_patterns(supported[0], filter_all, "*.bmp", NULL);
    
    supported[1] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[1], "JPEG (*.jpg, *.jpeg, *jpe)");
    file_filters_add_patterns(supported[1], filter_all, "*.jpg", "*.jpeg", "*.jpe", NULL);

    supported[2] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[2], "GIF (*.gif)");
    file_filters_add_patterns(supported[2], filter_all, "*.gif", NULL);
    
    supported[3] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[3], "PNG (*.png)");
    file_filters_add_patterns(supported[3], filter_all, "*.png", NULL);
    
    supported[4] = gtk_file_filter_new();
    gtk_file_filter_set_name(supported[4], "TIFF (*tif, *.tiff)");
    file_filters_add_patterns(supported[4], filter_all, "*.tiff", "*.tif", NULL);
    
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser_image), filter_all);
    for(int i = 0; i < 5; i++) {
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser_image), supported[i]);
    }
    
    if (settings->image_file != NULL) gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(chooser_image), settings->image_file);
    
    GtkWidget* separator2 = gtk_hseparator_new();
    vbox_position = gtk_vbox_new(FALSE, 5);
    
    hbox_opacity = gtk_hbox_new(FALSE, 5);
    label_opacity = gtk_label_new(g_strconcat(_("Opacity"), ":", NULL));
    gtk_misc_set_alignment(GTK_MISC(label_opacity), 0.5, 0.8);
    scale_opacity = gtk_hscale_new_with_range(1, 100, 1);
    gtk_widget_set_size_request (scale_opacity, 150, 50);
    gtk_range_set_value(GTK_RANGE(scale_opacity), settings->opacity);
    label_percent = gtk_label_new("%");
    gtk_misc_set_alignment(GTK_MISC(label_percent), 0.5, 0.8);
    
    hbox_edge = gtk_hbox_new(FALSE, 5);
    label_edge = gtk_label_new(g_strconcat(_("Distance to edge"), ":", NULL));
    spin_edge = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new (settings->edge_distance, 0, G_MAXINT, 1, 1, 0)), 1, 0);
    label_px = gtk_label_new("px");
    
    frame_position = gtk_frame_new(g_strconcat(_("Position on the image"), ":", NULL));
    table_position = gtk_table_new(3, 3, TRUE);
    gtk_widget_set_size_request (table_position, 250, 120);
    
    GtkRadioButton *first_button = NULL;
    for(watermark_position current_pos = WM_POS_TL; current_pos < WM_POS_END; current_pos++) {
        position_buttons[current_pos] = gtk_radio_button_new_from_widget(first_button);
        gtk_button_set_image(GTK_BUTTON(position_buttons[current_pos]), image_new_from_resource(g_strconcat("/gimp/plugin/bimp/icons/pos-", watermark_pos_get_abbreviation(current_pos), "-icon.png", NULL)));
        gtk_widget_set_tooltip_text (position_buttons[current_pos], watermark_pos_get_string(current_pos));
        gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(position_buttons[current_pos]), FALSE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(position_buttons[current_pos]), settings->position == current_pos);
        gtk_table_attach_defaults(GTK_TABLE(table_position), position_buttons[current_pos], current_pos % 3, (current_pos % 3) + 1, current_pos / 3, (current_pos / 3) + 1);
        
        if(first_button == NULL) {
            first_button = GTK_RADIO_BUTTON(position_buttons[current_pos]);
        }
    }
    
    // left side (text mode)
    gtk_box_pack_start(GTK_BOX(vbox_mode_text), radio_text, FALSE, FALSE, 0);
    
    gtk_table_attach(GTK_TABLE(table_text), label_text, 0, 1, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table_text), entry_text, 1, 2, 0, 1);
    gtk_table_attach(GTK_TABLE(table_text), label_font, 0, 1, 1, 2, GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table_text), chooser_font, 1, 2, 1, 2);
    gtk_table_attach(GTK_TABLE(table_text), label_color, 0, 1, 2, 3, GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table_text), chooser_color, 1, 2, 2, 3);
    
    gtk_container_add(GTK_CONTAINER(align_radio_text), table_text);
    gtk_box_pack_start(GTK_BOX(vbox_mode_text), align_radio_text, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox_mode), vbox_mode_text, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox_mode), separator1, FALSE, FALSE, 0);
    
    // right side (image mode)
    gtk_box_pack_start(GTK_BOX(vbox_mode_image), radio_image, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox_image), chooser_image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_image), check_image_adaptsize, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox_image_size), spin_image_sizepercent, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_image_size), label_percentof, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_image_size), combo_image_sizemode, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_image), hbox_image_size, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(align_radio_image), vbox_image);
    
    gtk_box_pack_start(GTK_BOX(vbox_mode_image), align_radio_image, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox_mode), vbox_mode_image, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(gui), hbox_mode, FALSE, FALSE, 0);
    
    // opacity
    gtk_box_pack_start(GTK_BOX(hbox_opacity), label_opacity, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_opacity), scale_opacity, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_opacity), label_percent, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_position), hbox_opacity, TRUE, TRUE, 0);
    
    // table
    gtk_container_add(GTK_CONTAINER(frame_position), table_position);
    gtk_box_pack_start(GTK_BOX(vbox_position), frame_position, FALSE, FALSE, 0);
    
    // distance to edge
    gtk_box_pack_start(GTK_BOX(hbox_edge), label_edge, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_edge), spin_edge, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_edge), label_px, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_position), hbox_edge, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(gui), separator2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), vbox_position, FALSE, FALSE, 0);
    
    toggle_group(NULL, NULL);
    toggle_image_size(NULL, NULL);
    
    g_signal_connect(G_OBJECT(radio_text), "toggled", G_CALLBACK(toggle_group), NULL);
    g_signal_connect(G_OBJECT(check_image_adaptsize), "toggled", G_CALLBACK(toggle_image_size), NULL);
    
    return gui;
}

static void toggle_group(GtkToggleButton *togglebutton, gpointer user_data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(table_text), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_text)));
    gtk_widget_set_sensitive(GTK_WIDGET(vbox_image), !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_text)));
}

static void toggle_image_size(GtkToggleButton *togglebutton, gpointer user_data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(hbox_image_size), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_image_adaptsize)));
}

static const char* watermark_pos_strings[9] =
{ 
    "Top-left", "Top-center", "Top-right",
    "Center-left", "Center", "Center-right",
    "Bottom-left", "Bottom-center", "Bottom-right"
};
static const char* watermark_pos_get_string(watermark_position wp) {
    return _(watermark_pos_strings[wp]);
}

static const char* watermark_pos_abbreviations[9] =
{
    "tl", "tc", "tr",
    "cl", "cc", "cr",
    "bl", "bc", "br"
};
static const char* watermark_pos_get_abbreviation(watermark_position wp) {
    return watermark_pos_abbreviations[wp];
}

static void file_filters_add_patterns(GtkFileFilter *filter1, GtkFileFilter *filter2, ...)
{
    va_list patterns;
    va_start(patterns, filter2);
    gchar *pattern = (gchar*)va_arg(patterns, void*);
    while (pattern != NULL) {
        gtk_file_filter_add_pattern (filter1, pattern);
        gtk_file_filter_add_pattern (filter2, pattern);
        pattern = (gchar*)va_arg(patterns, void*);
    }
}

void bimp_watermark_save(watermark_settings orig_settings) 
{    
    orig_settings->mode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_text));
    
    orig_settings->text = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_text)));
    orig_settings->font = pango_font_description_from_string (gtk_font_button_get_font_name(GTK_FONT_BUTTON(chooser_font)));
    gtk_color_button_get_color(GTK_COLOR_BUTTON(chooser_color), &(orig_settings->color));
    
    char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser_image));
    if (filename != NULL) {
        orig_settings->image_file = g_strdup(filename);
        g_free(filename);
    }
    
    watermark_image_sizemode new_mode = WM_IMG_NOSIZE;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_image_adaptsize))) {
        new_mode = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_image_sizemode)) == 0 ? WM_IMG_SIZEW : WM_IMG_SIZEH;
    }
    orig_settings->image_sizemode = new_mode;
    orig_settings->image_size_percent = gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_image_sizepercent));
    
    orig_settings->opacity = (float)gtk_range_get_value(GTK_RANGE(scale_opacity));
    orig_settings->edge_distance = gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_edge));
    
    for (watermark_position current_pos = WM_POS_TL; current_pos < WM_POS_END; current_pos++) {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(position_buttons[current_pos]))) {
            orig_settings->position = current_pos;
            break;
        }
    }
}
