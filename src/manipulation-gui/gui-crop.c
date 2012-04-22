#include <gtk/gtk.h>
#include "gui-crop.h"
#include "../bimp-manipulations.h"
#include "../bimp-manipulations-gui.h"
	
GtkWidget *radio_stratio, *radio_manual;
GtkWidget *combo_ratio;
GtkWidget *spin_width, *spin_height;
	
GtkWidget* bimp_crop_gui_new(crop_settings settings)
{
	GtkWidget *gui, *vbox_ratio, *vbox_manual, *hbox_manual_width, *hbox_manual_height;
	GtkWidget *label_manual_width, *label_manual_height;
	GtkWidget *align_radio_stratio, *align_radio_manual;
	
	gui = gtk_vbox_new(FALSE, 5);
	
	align_radio_stratio = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_stratio), 0, 5, 10, 0);
	
	radio_stratio = gtk_radio_button_new_with_label (NULL, "Crop to a standard aspect ratio");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_stratio), (!settings->manual));
	
	vbox_ratio = gtk_vbox_new(FALSE, 5);
	combo_ratio = gtk_combo_box_new_text();
	int i;
	for(i = 0; i < CROP_PRESET_END; i++) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo_ratio), crop_preset_string[i]);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_ratio), settings->ratio);
	
	align_radio_manual = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_manual), 0, 5, 10, 0);
	
	radio_manual = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radio_stratio), "Manual crop (pixel values)");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_manual), (settings->manual));
	
	vbox_manual = gtk_vbox_new(FALSE, 5);
	hbox_manual_width = gtk_hbox_new(FALSE, 5);
	label_manual_width = gtk_label_new("Width: ");
	spin_width = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new (settings->newW, 1, 40960, 1, 1, 0)), 1, 0);
	hbox_manual_height = gtk_hbox_new(FALSE, 5);
	label_manual_height = gtk_label_new("Height:");
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
	
	return gui;
}

void bimp_crop_save(crop_settings orig_settings) 
{
	orig_settings->newW = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
	orig_settings->newH = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
	orig_settings->manual = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_manual));
	orig_settings->ratio = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_ratio));
}








