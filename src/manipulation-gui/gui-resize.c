#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <libgimpwidgets/gimpwidgets.h>
#include "gui-resize.h"
#include "../bimp-manipulations.h"
#include "../plugin-intl.h"

static void toggle_units_group(GtkToggleButton*, gpointer);
static void toggle_resolution(GtkToggleButton*, gpointer);

GtkWidget *vbox_px, *hbox_res;
GtkWidget *radio_percent, *radio_px, *radio_px_both, *radio_px_width, *radio_px_height;
GtkWidget *spin_width, *spin_height, *spin_resX, *spin_resY;
GtkWidget *check_aspect, *check_resolution;
GtkWidget *label_unit;
GtkWidget *combo_quality;

gdouble last_percent_w_value;
gdouble last_percent_h_value;
gint last_pixel_w_value;
gint last_pixel_h_value;

gboolean previous_was_percent;

GtkWidget* bimp_resize_gui_new(resize_settings settings)
{
	GtkWidget *gui, *hbox_values, *hbox_quality;
	GtkWidget *label_width, *label_height, *label_quality, *label_resX, *label_resY, *label_dpi;
	GtkWidget *align_radio, *align_values, *align_res;
	
	last_percent_w_value = settings->new_w_pc;
	last_percent_h_value = settings->new_h_pc;
	last_pixel_w_value = settings->new_w_px;
	last_pixel_h_value = settings->new_h_px;
	
	gui = gtk_vbox_new(FALSE, 5);
	radio_percent = gtk_radio_button_new_with_label (NULL, _("Set to a percentage of the original"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_percent), (settings->resize_mode == RESIZE_PERCENT));
	radio_px = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio_percent), _("Set exact size in pixel"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_px), (settings->resize_mode != RESIZE_PERCENT));
	
	align_radio = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio), 0, 5, 10, 0);
	vbox_px = gtk_vbox_new(FALSE, 5);
	radio_px_both = gtk_radio_button_new_with_label (NULL, _("For both dimensions"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_px_both), (settings->resize_mode == RESIZE_PIXEL_BOTH));
	radio_px_width = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio_px_both), _("Set width only"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_px_width), (settings->resize_mode == RESIZE_PIXEL_WIDTH));
	radio_px_height = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio_px_both), _("Set height only"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_px_height), (settings->resize_mode == RESIZE_PIXEL_HEIGHT));
	
	check_aspect = gtk_check_button_new_with_label(_("Preserve aspect ratio"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_aspect), (settings->aspect_ratio));
	
	hbox_quality = gtk_hbox_new(FALSE, 5);
	label_quality = gtk_label_new(_("Interpolation quality"));
	combo_quality = gimp_enum_combo_box_new((GType)GIMP_TYPE_INTERPOLATION_TYPE);
	gimp_int_combo_box_set_active((GimpIntComboBox*)combo_quality, settings->interpolation);
		
	align_values = gtk_alignment_new(0.5, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_values), 5, 5, 0, 0);
	hbox_values = gtk_hbox_new(FALSE, 5);
	label_width = gtk_label_new(g_strconcat(_("Width"), ":", NULL));
	spin_width = gtk_spin_button_new(NULL, 1, 0);
	label_height = gtk_label_new(g_strconcat(_("Height"), ":", NULL));
	spin_height = gtk_spin_button_new(NULL, 1, 0);
	label_unit = gtk_label_new("<unit>");
	
	check_resolution = gtk_check_button_new_with_label(_("Change resolution"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_resolution), settings->change_res);
	align_res = gtk_alignment_new(0.5, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_res), 5, 5, 0, 0);
	hbox_res = gtk_hbox_new(FALSE, 5);
	label_resX = gtk_label_new(g_strconcat(_("X axis"), ":", NULL));
	spin_resX = gtk_spin_button_new(NULL, 1, 0);
	gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_resX), GTK_ADJUSTMENT(gtk_adjustment_new (settings->new_res_x, 0.005, 65536.000, 1.000, 1, 0)), 0, 3);
	label_resY = gtk_label_new(g_strconcat(_("Y axis"), ":", NULL));
	spin_resY = gtk_spin_button_new(NULL, 1, 0);
	gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_resY), GTK_ADJUSTMENT(gtk_adjustment_new (settings->new_res_y, 0.005, 65536.000, 1.000, 1, 0)), 0, 3);
	label_dpi = gtk_label_new("dpi");
	
	gtk_box_pack_start(GTK_BOX(vbox_px), radio_px_both, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_px), radio_px_width, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_px), radio_px_height, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(align_radio), vbox_px);
	
	gtk_box_pack_start(GTK_BOX(hbox_quality), label_quality, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_quality), combo_quality, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(hbox_values), label_width, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_values), spin_width, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_values), label_height, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_values), spin_height, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_values), label_unit, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(align_values), hbox_values);
	
	gtk_box_pack_start(GTK_BOX(hbox_res), label_resX, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_res), spin_resX, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_res), label_resY, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_res), spin_resY, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_res), label_dpi, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(align_res), hbox_res);
	
	gtk_box_pack_start(GTK_BOX(gui), radio_percent, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), radio_px, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), align_radio, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(gui), check_aspect, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), hbox_quality, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), align_values, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(gui), check_resolution, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), align_res, FALSE, FALSE, 0);
	
	previous_was_percent = FALSE;
	toggle_units_group(NULL, NULL);
	toggle_resolution(NULL, NULL);
	
	g_signal_connect(G_OBJECT(radio_percent), "toggled", G_CALLBACK(toggle_units_group), NULL);
	g_signal_connect(G_OBJECT(radio_px_both), "toggled", G_CALLBACK(toggle_units_group), NULL);
	g_signal_connect(G_OBJECT(radio_px_width), "toggled", G_CALLBACK(toggle_units_group), NULL);
	g_signal_connect(G_OBJECT(radio_px_height), "toggled", G_CALLBACK(toggle_units_group), NULL);
	g_signal_connect(G_OBJECT(check_aspect), "toggled", G_CALLBACK(toggle_units_group), NULL);
	g_signal_connect(G_OBJECT(check_resolution), "toggled", G_CALLBACK(toggle_resolution), NULL);
	
	return gui;
}

void toggle_units_group(GtkToggleButton *togglebutton, gpointer user_data) 
{
	gboolean percent_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_percent));
	gboolean both_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_px_both));
	gboolean width_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_px_width));
	gboolean height_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_px_height));
	gboolean aspect_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_aspect));
	
	if (percent_active) {
		/* store last pixel values */
		if (gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_width)) > 0 && gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_height)) > 0) {
			last_pixel_w_value = gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_width));
			last_pixel_h_value = gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_height));
		}
		
		gtk_widget_set_sensitive(GTK_WIDGET(vbox_px), FALSE);
		gtk_label_set_text(GTK_LABEL(label_unit), "%");
		
		/* set adjustments */
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
		
		gtk_widget_set_sensitive(GTK_WIDGET(vbox_px), TRUE);
		gtk_label_set_text(GTK_LABEL(label_unit), "px");
		
		gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_width), GTK_ADJUSTMENT(gtk_adjustment_new (last_pixel_w_value, 1, 262144, 1, 10, 0)), 0, 0);
		gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_height), GTK_ADJUSTMENT(gtk_adjustment_new (last_pixel_h_value, 1, 262144, 1, 10, 0)), 0, 0);
		
		previous_was_percent = FALSE;
	}
	
	/* adjustments for resolution spinners */
	if (aspect_active) {
		gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_resY), gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(spin_resX)), 0, 3);
	}
	else {
		gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_resY), GTK_ADJUSTMENT(gtk_adjustment_new (gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_resY)), 0.005, 65536.000, 1.000, 1, 0)), 0, 3);
	}
	
	gtk_widget_set_sensitive(GTK_WIDGET(spin_width), (percent_active || both_active || width_active));
	gtk_widget_set_sensitive(GTK_WIDGET(spin_height), (percent_active || both_active || height_active));
}

void toggle_resolution(GtkToggleButton *togglebutton, gpointer user_data) 
{
	gtk_widget_set_sensitive(GTK_WIDGET(hbox_res), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_resolution)));
}

void bimp_resize_save(resize_settings orig_settings) 
{	
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_percent))) {
		orig_settings->resize_mode = RESIZE_PERCENT;
		orig_settings->new_w_pc = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
		orig_settings->new_h_pc = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
		orig_settings->new_w_px = last_pixel_w_value;
		orig_settings->new_h_px = last_pixel_h_value;
	}
	else {
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_px_both))) {
			orig_settings->resize_mode = RESIZE_PIXEL_BOTH;
		}
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_px_width))) {
			orig_settings->resize_mode = RESIZE_PIXEL_WIDTH;
		}
		else {
			orig_settings->resize_mode = RESIZE_PIXEL_HEIGHT;
		}
		orig_settings->new_w_pc = last_percent_w_value;
		orig_settings->new_h_pc = last_percent_h_value;
		orig_settings->new_w_px = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
		orig_settings->new_h_px = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
	}
	
	orig_settings->aspect_ratio = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_aspect));
	
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

