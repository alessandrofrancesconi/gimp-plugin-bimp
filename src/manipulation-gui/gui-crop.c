#include <gtk/gtk.h>
#include "gui-crop.h"
#include "../bimp-manipulations.h"
#include "../bimp-manipulations-gui.h"
#include "../plugin-intl.h"
    
static void toggle_group(GtkToggleButton*, gpointer);
static void set_customratio(GtkComboBox*, gpointer);
static char* crop_preset_get_string(crop_preset);

static GtkWidget *hbox_ratio, *table_manual, *hbox_customratio;
static GtkWidget *radio_stratio, *radio_manual;
static GtkWidget *combo_ratio, *spin_ratio1, *spin_ratio2;
static GtkWidget *spin_width, *spin_height;
static GtkWidget *hbox_startpos, *combo_startpos;

GtkWidget* bimp_crop_gui_new(crop_settings settings)
{
    GtkWidget *gui;
    GtkWidget *label_manual_width, *label_manual_height, *label_manual_ratio, *label_startpos;
    GtkWidget *align_radio_stratio, *align_radio_manual;
    
    gui = gtk_vbox_new(FALSE, 5);
    
    align_radio_stratio = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_stratio), 0, 5, 10, 0);
    
    radio_stratio = gtk_radio_button_new_with_label (NULL, _("Crop to a standard aspect ratio"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_stratio), (!settings->manual));
    
    hbox_ratio = gtk_hbox_new(FALSE, 5);
    combo_ratio = gtk_combo_box_new_text();
    int i;
    for(i = 0; i < CROP_PRESET_END; i++) {
        gtk_combo_box_append_text(GTK_COMBO_BOX(combo_ratio), crop_preset_get_string(i));
    }
    
    hbox_customratio = gtk_hbox_new(FALSE, 5);
    spin_ratio1 = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new (settings->custom_ratio1, 0.1, 100.0, 0.1, 1, 0)), 1, 1);
    label_manual_ratio = gtk_label_new(":");
    spin_ratio2 = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new (settings->custom_ratio2, 0.1, 100.0, 0.1, 1, 0)), 1, 1);
    
    gtk_box_pack_start(GTK_BOX(hbox_customratio), spin_ratio1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_customratio), label_manual_ratio, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_customratio), spin_ratio2, FALSE, FALSE, 0);
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_ratio), settings->ratio);
    
    align_radio_manual = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_manual), 0, 5, 10, 0);
    
    radio_manual = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radio_stratio), _("Manual crop (pixel values)"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_manual), (settings->manual));
    
    table_manual = gtk_table_new(2, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table_manual), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table_manual), 5);
    label_manual_width = gtk_label_new(g_strconcat(_("Width"), ": ", NULL));
    gtk_misc_set_alignment(GTK_MISC(label_manual_width), 0, .5);
    spin_width = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new (settings->new_w, 1, 40960, 1, 1, 0)), 1, 0);
    label_manual_height = gtk_label_new(g_strconcat(_("Height"), ": ", NULL));
    gtk_misc_set_alignment(GTK_MISC(label_manual_height), 0, .5);
    spin_height = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new (settings->new_h, 1, 40960, 1, 1, 0)), 1, 0);
    
    hbox_startpos = gtk_hbox_new(FALSE, 5);
    label_startpos = gtk_label_new(g_strconcat(_("Start from"), ":", NULL));
    combo_startpos = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_startpos), _("Center"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_startpos), _("Top-left"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_startpos), _("Top-right"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_startpos), _("Bottom-left"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_startpos), _("Bottom-right"));
    
    int active_index;
    switch(settings->start_pos) {
        case CROP_START_CC: active_index = 0; break;
        case CROP_START_TL: active_index = 1; break;
        case CROP_START_TR: active_index = 2; break;
        case CROP_START_BL: active_index = 3; break;
        case CROP_START_BR: active_index = 4; break;
        default: active_index = 0; break;
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_startpos), active_index);
    
    gtk_box_pack_start(GTK_BOX(gui), radio_stratio, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox_ratio), combo_ratio, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_ratio), hbox_customratio, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(align_radio_stratio), hbox_ratio);
    gtk_box_pack_start(GTK_BOX(gui), align_radio_stratio, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(gui), radio_manual, FALSE, FALSE, 0);
    
    gtk_table_attach(GTK_TABLE(table_manual), label_manual_width, 0, 1, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table_manual), spin_width, 1, 2, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table_manual), label_manual_height, 0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table_manual), spin_height, 1, 2, 1, 2);
    gtk_container_add(GTK_CONTAINER(align_radio_manual), table_manual);
    gtk_box_pack_start(GTK_BOX(gui), align_radio_manual, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox_startpos), label_startpos, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_startpos), combo_startpos, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), hbox_startpos, FALSE, FALSE, 0);
    
    toggle_group(NULL, NULL);
    set_customratio(NULL, NULL);
    g_signal_connect(G_OBJECT(radio_stratio), "toggled", G_CALLBACK(toggle_group), NULL);
    g_signal_connect(G_OBJECT(combo_ratio), "changed", G_CALLBACK(set_customratio), NULL);
    
    return gui;
}

static void toggle_group(GtkToggleButton *togglebutton, gpointer user_data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(hbox_ratio), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_stratio)));
    gtk_widget_set_sensitive(GTK_WIDGET(table_manual), !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_stratio)));
}

static void set_customratio(GtkComboBox *combobox, gpointer user_data)
{
    gtk_widget_set_sensitive(hbox_customratio, gtk_combo_box_get_active(GTK_COMBO_BOX(combo_ratio)) == CROP_PRESET_CUSTOM);
}

static char* crop_preset_get_string(crop_preset cps) {
    char* crop_string = "";
    
    if (cps == CROP_PRESET_11) {
        crop_string = g_strconcat(_("One-to-one"), " (1:1)", NULL);
    }
    else if (cps == CROP_PRESET_32) {
        crop_string = g_strconcat(_("Classic 35 mm film"), " (3:2)", NULL);
    }
    else if (cps == CROP_PRESET_43) {
        crop_string = g_strconcat(_("Standard VGA monitor"), " (4:3)", NULL);
    }
    else if (cps == CROP_PRESET_169) {
        crop_string = g_strconcat(_("Widescreen"), " (16:9)", NULL);
    }
    else if (cps == CROP_PRESET_1610) {
        crop_string = g_strconcat(_("Widescreen extended"), " (16:10)", NULL);
    }
    else if (cps == CROP_PRESET_EUPORT) {
        crop_string = g_strconcat(_("EU Passport portrait"), " (7:9)", NULL);
    }
    else if (cps == CROP_PRESET_PHONE) {
        crop_string = g_strconcat(_("Classic smartphone screen"), " (2:3)", NULL);
    }
    else if (cps == CROP_PRESET_TALLPHONE) {
        crop_string = g_strconcat(_("Tall smartphone screen"), " (40:71)", NULL);
    }
    else if (cps == CROP_PRESET_TABLET) {
        crop_string = g_strconcat(_("Classic tablet screen"), " (3:4)", NULL);
    }
    else if (cps == CROP_PRESET_CUSTOM) {
        crop_string = g_strconcat(_("Custom ratio"), "...", NULL);
    }
    
    return crop_string;
}

void bimp_crop_save(crop_settings orig_settings) 
{
    orig_settings->new_w = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
    orig_settings->new_h = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
    orig_settings->manual = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_manual));
    orig_settings->ratio = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_ratio));
    orig_settings->custom_ratio1 = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_ratio1));
    orig_settings->custom_ratio2 = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_ratio2));
    orig_settings->start_pos = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_startpos));
}








