#include <gtk/gtk.h>
#include "gui-crop.h"
#include "../bimp-manipulations.h"
#include "../bimp-manipulations-gui.h"
#include "../plugin-intl.h"
	
static void toggle_group(GtkToggleButton*, gpointer);
static char* crop_preset_get_string(crop_preset);
	
GtkWidget *vbox_ratio, *vbox_manual;
GtkWidget *radio_stratio, *radio_manual;
GtkWidget *combo_ratio;
GtkWidget *spin_width, *spin_height;
	
GtkWidget* bimp_crop_gui_new(crop_settings settings)
{
	GtkWidget *gui, *hbox_manual_width, *hbox_manual_height;
	GtkWidget *label_manual_width, *label_manual_height;
	GtkWidget *align_radio_stratio, *align_radio_manual;
	
	gui = gtk_vbox_new(FALSE, 5);
	
	align_radio_stratio = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_stratio), 0, 5, 10, 0);
	
	radio_stratio = gtk_radio_button_new_with_label (NULL, _("Crop to a standard aspect ratio"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_stratio), (!settings->manual));
	
	vbox_ratio = gtk_vbox_new(FALSE, 5);
	combo_ratio = gtk_combo_box_new_text();
	int i;
	for(i = 0; i < CROP_PRESET_END; i++) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo_ratio), crop_preset_get_string(i));
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_ratio), settings->ratio);
	
	align_radio_manual = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_manual), 0, 5, 10, 0);
	
	radio_manual = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radio_stratio), _("Manual crop (pixel values)"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_manual), (settings->manual));
	
	vbox_manual = gtk_vbox_new(FALSE, 5);
	hbox_manual_width = gtk_hbox_new(FALSE, 5);
	label_manual_width = gtk_label_new(g_strconcat(_("Width"), ": ", NULL));
	gtk_widget_set_size_request (label_manual_width, LABEL_CROP_W, LABEL_CROP_H);
	spin_width = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new (settings->newW, 1, 40960, 1, 1, 0)), 1, 0);
	hbox_manual_height = gtk_hbox_new(FALSE, 5);
	label_manual_height = gtk_label_new(g_strconcat(_("Height"), ": ", NULL));
	gtk_widget_set_size_request (label_manual_height, LABEL_CROP_W, LABEL_CROP_H);
	spin_height = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new (settings->newH, 1, 40960, 1, 1, 0)), 1, 0);
	
	gtk_box_pack_start(GTK_BOX(gui), radio_stratio, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(vbox_ratio), combo_ratio, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(align_radio_stratio), vbox_ratio);
	gtk_box_pack_start(GTK_BOX(gui), align_radio_stratio, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(gui), radio_manual, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(hbox_manual_width), label_manual_width, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_manual_width), spin_width, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_manual_height), label_manual_height, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_manual_height), spin_height, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(vbox_manual), hbox_manual_width, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_manual), hbox_manual_height, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(align_radio_manual), vbox_manual);
	gtk_box_pack_start(GTK_BOX(gui), align_radio_manual, FALSE, FALSE, 0);
	
	toggle_group(NULL, NULL);
	g_signal_connect(G_OBJECT(radio_stratio), "toggled", G_CALLBACK(toggle_group), NULL);
	
	return gui;
}

static void toggle_group(GtkToggleButton *togglebutton, gpointer user_data)
{
	gtk_widget_set_sensitive(GTK_WIDGET(vbox_ratio), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_stratio)));
	gtk_widget_set_sensitive(GTK_WIDGET(vbox_manual), !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_stratio)));
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
	else if (cps == CROP_PRESET_TABLET) {
		crop_string = g_strconcat(_("Classic tablet screen"), " (3:4)", NULL);
	}
	
	return crop_string;
}

void bimp_crop_save(crop_settings orig_settings) 
{
	orig_settings->newW = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
	orig_settings->newH = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
	orig_settings->manual = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_manual));
	orig_settings->ratio = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_ratio));
}








