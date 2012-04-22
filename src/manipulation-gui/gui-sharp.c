#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include "gui-sharp.h"
#include "../bimp-manipulations.h"

GtkWidget *scale_sharp;

GtkWidget* bimp_sharp_gui_new(sharp_settings settings)
{
	GtkWidget *gui, *hbox_sharp;
	GtkWidget *label_scale, *label_percent;
	
	gui = gtk_vbox_new(FALSE, 5);
	
	hbox_sharp = gtk_hbox_new(FALSE, 5);
	label_scale = gtk_label_new("Amount:");
	gtk_widget_set_size_request (label_scale, LABEL_AMOUNT_W, LABEL_AMOUNT_H);
	gtk_misc_set_alignment(GTK_MISC(label_scale), 0.5, 0.8);
	scale_sharp = gtk_hscale_new_with_range(1, 100, 1);
	gtk_range_set_value(GTK_RANGE(scale_sharp), settings->amount);
	gtk_widget_set_size_request (scale_sharp, SCALE_AMOUNT_W, SCALE_AMOUNT_H);
	label_percent = gtk_label_new("%");
	gtk_widget_set_size_request (label_percent, LABEL_PERCENT_W, LABEL_PERCENT_H);
	gtk_misc_set_alignment(GTK_MISC(label_percent), 0.5, 0.8);
	
	gtk_box_pack_start(GTK_BOX(hbox_sharp), label_scale, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_sharp), scale_sharp, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_sharp), label_percent, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(gui), hbox_sharp, FALSE, FALSE, 0);
	
	return gui;
}

void bimp_sharp_save(sharp_settings orig_settings) 
{
	orig_settings->amount = (int)gtk_range_get_value(GTK_RANGE(scale_sharp));
}

