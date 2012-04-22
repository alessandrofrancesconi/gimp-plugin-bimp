#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <libgimpwidgets/gimpwidgets.h>
#include "gui-resize.h"
#include "../bimp-manipulations.h"

/* ok... it works.... BUT IT'S PROBABLY THE WORST CODE I'VE EVER WROTE .-. */

static void toggle_units_group(GtkToggleButton*, gpointer);

GtkWidget *vbox_px;
GtkWidget *radio_percent, *radio_px, *radio_px_both, *radio_px_width, *radio_px_height;
GtkWidget *spin_width, *spin_height;
GtkWidget *check_aspect;
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
	GtkWidget *label_width, *label_height, *label_quality;
	GtkWidget *align_radio, *align_values;
	
	last_percent_w_value = settings->newWpc;
	last_percent_h_value = settings->newHpc;
	last_pixel_w_value = settings->newWpx;
	last_pixel_h_value = settings->newHpx;
	
	gui = gtk_vbox_new(FALSE, 5);
	radio_percent = gtk_radio_button_new_with_label (NULL, "Set to a percentage of the original");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_percent), (settings->sizemode == RESIZE_PERCENT));
	radio_px = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio_percent), "Set exact sizes in pixel");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_px), (settings->sizemode != RESIZE_PERCENT));
	
	align_radio = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio), 0, 5, 10, 0);
	vbox_px = gtk_vbox_new(FALSE, 5);
	radio_px_both = gtk_radio_button_new_with_label (NULL, "For both dimensions");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_px_both), (settings->sizemode == RESIZE_PIXEL_BOTH));
	radio_px_width = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio_px_both), "Set width only");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_px_width), (settings->sizemode == RESIZE_PIXEL_WIDTH));
	radio_px_height = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio_px_both), "Set height only");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_px_height), (settings->sizemode == RESIZE_PIXEL_HEIGHT));
	
	check_aspect = gtk_check_button_new_with_label("Preserve aspect ratio");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_aspect), (settings->aspect_ratio));
	
	hbox_quality = gtk_hbox_new(FALSE, 5);
	label_quality = gtk_label_new("Interpolation quality");
	combo_quality = gimp_enum_combo_box_new((GType)GIMP_TYPE_INTERPOLATION_TYPE);
	gimp_int_combo_box_set_active((GimpIntComboBox*)combo_quality, settings->interpolation);
		
	align_values = gtk_alignment_new(0.5, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_values), 5, 5, 0, 0);
	hbox_values = gtk_hbox_new(FALSE, 5);
	label_width = gtk_label_new("Width:");
	spin_width = gtk_spin_button_new(NULL, 1, 0);
	label_height = gtk_label_new("Height:");
	spin_height = gtk_spin_button_new(NULL, 1, 0);
	label_unit = gtk_label_new("<unit>");
	
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
	
	gtk_box_pack_start(GTK_BOX(gui), radio_percent, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), radio_px, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), align_radio, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), check_aspect, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), hbox_quality, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), align_values, FALSE, FALSE, 0);
	
	previous_was_percent = FALSE;
	toggle_units_group(NULL, NULL);
	
	g_signal_connect(G_OBJECT(radio_percent), "toggled", G_CALLBACK(toggle_units_group), NULL);
	g_signal_connect(G_OBJECT(radio_px_both), "toggled", G_CALLBACK(toggle_units_group), NULL);
	g_signal_connect(G_OBJECT(radio_px_width), "toggled", G_CALLBACK(toggle_units_group), NULL);
	g_signal_connect(G_OBJECT(radio_px_height), "toggled", G_CALLBACK(toggle_units_group), NULL);
	g_signal_connect(G_OBJECT(check_aspect), "toggled", G_CALLBACK(toggle_units_group), NULL);
	
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
		gtk_label_set_text(GTK_LABEL(label_unit), "Px");
		
		gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_width), GTK_ADJUSTMENT(gtk_adjustment_new (last_pixel_w_value, 1, 262144, 1, 10, 0)), 0, 0);
		gtk_spin_button_configure (GTK_SPIN_BUTTON(spin_height), GTK_ADJUSTMENT(gtk_adjustment_new (last_pixel_h_value, 1, 262144, 1, 10, 0)), 0, 0);
		
		previous_was_percent = FALSE;
	}
	
	gtk_widget_set_sensitive(GTK_WIDGET(check_aspect), (percent_active || !both_active));
	gtk_widget_set_sensitive(GTK_WIDGET(spin_width), (percent_active || both_active || width_active));
	gtk_widget_set_sensitive(GTK_WIDGET(spin_height), (percent_active || both_active || height_active));	
}

void bimp_resize_save(resize_settings orig_settings) 
{	
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_percent))) {
		orig_settings->sizemode = RESIZE_PERCENT;
		orig_settings->newWpc = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
		orig_settings->newHpc = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
		orig_settings->newWpx = last_pixel_w_value;
		orig_settings->newHpx = last_pixel_h_value;
	}
	else {
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_px_both))) {
			orig_settings->sizemode = RESIZE_PIXEL_BOTH;
		}
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_px_width))) {
			orig_settings->sizemode = RESIZE_PIXEL_WIDTH;
		}
		else {
			orig_settings->sizemode = RESIZE_PIXEL_HEIGHT;
		}
		orig_settings->newWpc = last_percent_w_value;
		orig_settings->newHpc = last_percent_h_value;
		orig_settings->newWpx = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_width));
		orig_settings->newHpx = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_height));
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
}

