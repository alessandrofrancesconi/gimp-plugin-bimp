#include <gtk/gtk.h>
#include "gui-crop.h"
#include "../bimp-manipulations.h"
#include "../bimp-manipulations-gui.h"
#include "../plugin-intl.h"

static void toggle_group(GtkToggleButton*, gpointer);
static void set_customratio(GtkComboBox*, gpointer);
static char* crop_preset_get_string(crop_preset);

GtkWidget *hbox_ratio, *vbox_manual, *hbox_customratio, *vbox_custom_anchor;
GtkWidget *radio_stratio, *radio_manual, *check_custom_anchor;
GtkWidget *combo_ratio, *spin_ratio1, *spin_ratio2;
GtkWidget *spin_width, *spin_height, *spin_anchor_x, *spin_anchor_y;

GtkWidget* bimp_crop_gui_new(crop_settings settings)
{
	GtkWidget *gui, *hbox_manual_width, *hbox_manual_height, *hbox_anchor_x, *hbox_anchor_y;
	GtkWidget	*label_manual_width, *label_manual_height, *label_manual_ratio,
		*label_manual_anchor_x, *label_manual_anchor_y;
	GtkWidget *align_radio_stratio, *align_radio_manual, *align_check_custom_anchor;

	gui = gtk_vbox_new(FALSE, 5);

	align_radio_stratio = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_stratio), 0, 5, 10, 0);

	radio_stratio = gtk_radio_button_new_with_label(NULL, _("Crop to a standard aspect ratio"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_stratio), (!settings->manual));

	hbox_ratio = gtk_hbox_new(FALSE, 5);
	combo_ratio = gtk_combo_box_new_text();
	int i;
	for (i = 0; i < CROP_PRESET_END; i++) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo_ratio), crop_preset_get_string(i));
	}

	hbox_customratio = gtk_hbox_new(FALSE, 5);
	spin_ratio1 = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(settings->custom_ratio1, 0.1, 100.0, 0.1, 1, 0)), 1, 1);
	gtk_widget_set_size_request(spin_ratio1, 50, LABEL_CROP_H);
	label_manual_ratio = gtk_label_new(":");
	spin_ratio2 = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(settings->custom_ratio2, 0.1, 100.0, 0.1, 1, 0)), 1, 1);
	gtk_widget_set_size_request(spin_ratio2, 50, LABEL_CROP_H);

	gtk_box_pack_start(GTK_BOX(hbox_customratio), spin_ratio1, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_customratio), label_manual_ratio, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_customratio), spin_ratio2, FALSE, FALSE, 0);

	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_ratio), settings->ratio);

	align_radio_manual = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_manual), 0, 5, 10, 0);

	radio_manual = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_stratio), _("Manual crop (pixel values)"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_manual), (settings->manual));

	vbox_manual = gtk_vbox_new(FALSE, 5);
	hbox_manual_width = gtk_hbox_new(FALSE, 5);
	label_manual_width = gtk_label_new(g_strconcat(_("Width"), ": ", NULL));
	gtk_widget_set_size_request(label_manual_width, LABEL_CROP_W, LABEL_CROP_H);
	spin_width = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(settings->new_w, 1, 40960, 1, 1, 0)), 1, 0);
	hbox_manual_height = gtk_hbox_new(FALSE, 5);
	label_manual_height = gtk_label_new(g_strconcat(_("Height"), ": ", NULL));
	gtk_widget_set_size_request(label_manual_height, LABEL_CROP_W, LABEL_CROP_H);
	spin_height = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(settings->new_h, 1, 40960, 1, 1, 0)), 1, 0);

	align_check_custom_anchor = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_check_custom_anchor), 0, 5, 10, 0);

	check_custom_anchor = gtk_check_menu_item_new_with_label(_("Set upper left corner anchor for cropped image:"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_custom_anchor), (settings->custom_anchor));

	hbox_anchor_x = gtk_hbox_new(FALSE, 5);
	label_manual_anchor_x = gtk_label_new(g_strconcat(_("X Anchor"), ": ", NULL));
	gtk_widget_set_size_request(label_manual_anchor_x, LABEL_ANCHOR_W, LABEL_ANCHOR_H);
	spin_anchor_x = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(settings->anchor_x, 1, 40960, 1, 1, 0)), 1, 0);
	hbox_anchor_y = gtk_hbox_new(FALSE, 5);
	label_manual_anchor_y = gtk_label_new(g_strconcat(_("Y Anchor"), ": ", NULL));
	gtk_widget_set_size_request(label_manual_anchor_y, LABEL_ANCHOR_W, LABEL_ANCHOR_H);
	spin_anchor_y = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(settings->anchor_y, 1, 40960, 1, 1, 0)), 1, 0);


	gtk_box_pack_start(GTK_BOX(gui), radio_stratio, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(hbox_ratio), combo_ratio, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_ratio), hbox_customratio, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(align_radio_stratio), hbox_ratio);
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

	gtk_box_pack_start(GTK_BOX(gui), check_custom_anchor, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox_custom_anchor), label_manual_anchor_x, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_custom_anchor), spin_anchor_x, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_custom_anchor), label_manual_anchor_y, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_custom_anchor), spin_anchor_y, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox_custom_anchor), hbox_manual_anchor_x, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_custom_anchor), hbox_manual_anchor_y, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(align_check_custom_anchor), vbox_custom_anchor);
	gtk_box_pack_start(GTK_BOX(gui), align_check_custom_anchor, FALSE, FALSE, 0);

	toggle_group(NULL, NULL);
	set_customratio(NULL, NULL);
	g_signal_connect(G_OBJECT(radio_stratio), "toggled", G_CALLBACK(toggle_group), NULL);
	g_signal_connect(G_OBJECT(combo_ratio), "changed", G_CALLBACK(set_customratio), NULL);

	return gui;
}

static void toggle_group(GtkToggleButton *togglebutton, gpointer user_data)
{
	gtk_widget_set_sensitive(GTK_WIDGET(hbox_ratio), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_stratio)));
	gtk_widget_set_sensitive(GTK_WIDGET(vbox_manual), !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_stratio)));
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
	orig_settings->anchor_x = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_anchor_x));
	orig_settings->anchor_y = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_anchor_y));
	orig_settings->new_w = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
	orig_settings->new_h = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
	orig_settings->manual = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_manual));
	orig_settings->ratio = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_ratio));
	orig_settings->custom_ratio1 = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_ratio1));
	orig_settings->custom_ratio2 = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_ratio2));
	orig_settings->custom_anchor = gtk_check_button_get_value(GTK_CHECK_BUTTON(check_custom_anchor));
}