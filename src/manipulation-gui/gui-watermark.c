#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "gui-watermark.h"
#include "../bimp-manipulations.h"
#include "../bimp-manipulations-gui.h"

static void toggle_group(GtkToggleButton*, gpointer);

GtkWidget *vbox_text, *vbox_image;
GtkWidget *radio_text, *radio_image;
GtkWidget *entry_text;
GtkWidget *chooser_font, *chooser_color, *chooser_image;
GtkWidget *scale_opacity;
GtkWidget *button_tl, *button_tr, *button_cc, *button_bl, *button_br;

GtkWidget* bimp_watermark_gui_new(watermark_settings settings)
{
	GtkWidget *gui, *hbox_text_entry, *hbox_text_font, *hbox_text_color, *hbox_opacity, *table_position;
	GtkWidget *align_radio_text, *align_radio_image;
	GtkWidget *label_text, *label_font, *label_color, *label_opacity, *label_percent, *label_position;
	
	gui = gtk_vbox_new(FALSE, 5);
	
	align_radio_text = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_text), 0, 5, 10, 0);
	radio_text = gtk_radio_button_new_with_label(NULL, "Text watermark");
	
	vbox_text = gtk_vbox_new(FALSE, 5);
	hbox_text_entry = gtk_hbox_new(FALSE, 5);
	label_text = gtk_label_new("Text:");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_text), settings->textmode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_text), settings->textmode);
	entry_text = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_text), 50);
	gtk_entry_set_text(GTK_ENTRY(entry_text), settings->text);
	gtk_widget_set_size_request (entry_text, BUTTON_W, BUTTON_H);
	
	hbox_text_font = gtk_hbox_new(FALSE, 5);
	label_font = gtk_label_new("Font:");
	chooser_font = gtk_font_button_new_with_font(pango_font_description_to_string(settings->font));
	gtk_widget_set_size_request (chooser_font, BUTTON_W, BUTTON_H);
	
	hbox_text_color = gtk_hbox_new(FALSE, 5);
	label_color = gtk_label_new("Color:");
	chooser_color = gtk_color_button_new_with_color(&(settings->color));
	gtk_widget_set_size_request (chooser_color, BUTTON_W, BUTTON_H);
	
	align_radio_image = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_image), 0, 5, 10, 0);
	radio_image = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radio_text), "Image watermark");
	
	vbox_image = gtk_vbox_new(FALSE, 5);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_image), !settings->textmode);
	chooser_image = gtk_file_chooser_button_new("Select image", GTK_FILE_CHOOSER_ACTION_OPEN);
	if (settings->imagefile != NULL) {
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(chooser_image), settings->imagefile);
	}
	gtk_widget_set_size_request (chooser_image, BUTTON_W, BUTTON_H);
	
	hbox_opacity = gtk_hbox_new(FALSE, 5);
	label_opacity = gtk_label_new("Opacity:");
	gtk_widget_set_size_request (label_opacity, LABEL_TRANSP_W, LABEL_TRANSP_H);
	gtk_misc_set_alignment(GTK_MISC(label_opacity), 0.5, 0.8);
	scale_opacity = gtk_hscale_new_with_range(1, 100, 1);
	gtk_range_set_value(GTK_RANGE(scale_opacity), settings->opacity);
	gtk_widget_set_size_request (scale_opacity, SCALE_TRANSP_W, SCALE_TRANSP_H);
	label_percent = gtk_label_new("%");
	gtk_widget_set_size_request (label_percent, LABEL_PERCENT_W, LABEL_PERCENT_H);
	gtk_misc_set_alignment(GTK_MISC(label_percent), 0.5, 0.8);
	
	label_position = gtk_label_new("Position:");
	gtk_misc_set_alignment(GTK_MISC(label_position), 0, 0.5);
	table_position = gtk_table_new(3, 3, TRUE);
	
	button_tl = gtk_radio_button_new_with_label (NULL, watermark_position_string[WM_POS_TL]);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button_tl), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_tl), settings->position == WM_POS_TL);
	gtk_table_attach(GTK_TABLE(table_position), button_tl, 0, 1, 0, 1, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 0, 0);
	
	button_tr = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(button_tl), watermark_position_string[WM_POS_TR]);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button_tr), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_tr), settings->position == WM_POS_TR);
	gtk_table_attach(GTK_TABLE(table_position), button_tr, 2, 3, 0, 1, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 0, 0);
	
	button_cc = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(button_tl), watermark_position_string[WM_POS_CC]);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button_cc), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_cc), settings->position == WM_POS_CC);
	gtk_table_attach(GTK_TABLE(table_position), button_cc, 1, 2, 1, 2, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 0, 0);
	
	button_bl = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(button_tl), watermark_position_string[WM_POS_BL]);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button_bl), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_bl), settings->position == WM_POS_BL);
	gtk_table_attach(GTK_TABLE(table_position), button_bl, 0, 1, 2, 3, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 0, 0);
	
	button_br = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(button_tl), watermark_position_string[WM_POS_BR]);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button_br), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_br), settings->position == WM_POS_BR);
	gtk_table_attach(GTK_TABLE(table_position), button_br, 2, 3, 2, 3, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 0, 0);
	
	gtk_box_pack_start(GTK_BOX(gui), radio_text, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(hbox_text_entry), label_text, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_text_entry), entry_text, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_text_font), label_font, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_text_font), chooser_font, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_text_color), label_color, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_text_color), chooser_color, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(vbox_text), hbox_text_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_text), hbox_text_font, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_text), hbox_text_color, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(align_radio_text), vbox_text);
	gtk_box_pack_start(GTK_BOX(gui), align_radio_text, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(gui), radio_image, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(vbox_image), chooser_image, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(align_radio_image), vbox_image);
	gtk_box_pack_start(GTK_BOX(gui), align_radio_image, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(hbox_opacity), label_opacity, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_opacity), scale_opacity, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_opacity), label_percent, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(gui), hbox_opacity, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), label_position, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), table_position, FALSE, FALSE, 0);
	
	toggle_group(NULL, NULL);
	
	g_signal_connect(G_OBJECT(radio_text), "toggled", G_CALLBACK(toggle_group), NULL);
	
	return gui;
}

void toggle_group(GtkToggleButton *togglebutton, gpointer user_data)
{
	gtk_widget_set_sensitive(GTK_WIDGET(vbox_text), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_text)));
	gtk_widget_set_sensitive(GTK_WIDGET(vbox_image), !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_text)));
}

void bimp_watermark_save(watermark_settings orig_settings) 
{	
	orig_settings->textmode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_text));
	
	if (gtk_entry_get_text_length(GTK_ENTRY(entry_text)) > 0) {
		strcpy(orig_settings->text, gtk_entry_get_text(GTK_ENTRY(entry_text)));
	}
	
	char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser_image));
	if (filename != NULL) {
		free(orig_settings->imagefile);
		orig_settings->imagefile = (char*)malloc(sizeof(char)*(strlen(filename) + 1));
		strcpy(orig_settings->imagefile, filename);
		
		g_free(filename);
	}
	
	orig_settings->font = pango_font_description_from_string (gtk_font_button_get_font_name(GTK_FONT_BUTTON(chooser_font)));
	gtk_color_button_get_color(GTK_COLOR_BUTTON(chooser_color), &(orig_settings->color));
	orig_settings->opacity = (float)gtk_range_get_value(GTK_RANGE(scale_opacity));
	
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_tr))) {
		orig_settings->position = WM_POS_TR;
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_tl))) {
		orig_settings->position = WM_POS_TL;
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_cc))) {
		orig_settings->position = WM_POS_CC;
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_br))) {
		orig_settings->position = WM_POS_BR;
	}
	else {
		orig_settings->position = WM_POS_BL;
	}
}








