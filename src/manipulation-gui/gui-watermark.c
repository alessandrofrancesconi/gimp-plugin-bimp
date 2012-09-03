#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "gui-watermark.h"
#include "../bimp-manipulations.h"
#include "../bimp-manipulations-gui.h"
#include "../bimp-icons.h"

static void toggle_group(GtkToggleButton*, gpointer);

GtkWidget *vbox_text, *vbox_image;
GtkWidget *radio_text, *radio_image;
GtkWidget *entry_text;
GtkWidget *chooser_font, *chooser_color, *chooser_image;
GtkWidget *scale_opacity;
GtkWidget *button_tl, *button_tr, *button_cc, *button_bl, *button_br;

GtkWidget* bimp_watermark_gui_new(watermark_settings settings)
{
	GtkWidget *gui, *hbox_text_entry, *hbox_text_font, *hbox_text_color, *hbox_opacity, *frame_position, *table_position;
	GtkWidget *align_radio_text, *align_radio_image;
	GtkWidget *label_text, *label_font, *label_color, *label_opacity, *label_percent;
		
	gui = gtk_vbox_new(FALSE, 5);
	
	align_radio_text = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_text), 0, 5, 10, 0);
	radio_text = gtk_radio_button_new_with_label(NULL, "Text watermark");
	
	vbox_text = gtk_vbox_new(FALSE, 5);
	hbox_text_entry = gtk_hbox_new(FALSE, 5);
	label_text = gtk_label_new("Text:");
	gtk_widget_set_size_request (label_text, LABEL_W, LABEL_H);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_text), settings->textmode);
	entry_text = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_text), 50);
	gtk_entry_set_text(GTK_ENTRY(entry_text), settings->text);
	gtk_widget_set_size_request (entry_text, INPUT_W, INPUT_H);
	
	hbox_text_font = gtk_hbox_new(FALSE, 5);
	label_font = gtk_label_new("Font:");
	gtk_widget_set_size_request (label_font, LABEL_W, LABEL_H);
	chooser_font = gtk_font_button_new_with_font(pango_font_description_to_string(settings->font));
	gtk_widget_set_size_request (chooser_font, INPUT_W, INPUT_H);
	
	hbox_text_color = gtk_hbox_new(FALSE, 5);
	label_color = gtk_label_new("Color:");
	gtk_widget_set_size_request (label_color, LABEL_W, LABEL_H);
	chooser_color = gtk_color_button_new_with_color(&(settings->color));
	gtk_widget_set_size_request (chooser_color, INPUT_W, INPUT_H);
	
	align_radio_image = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align_radio_image), 0, 5, 10, 0);
	radio_image = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radio_text), "Image watermark");
	
	vbox_image = gtk_vbox_new(FALSE, 5);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_image), !settings->textmode);
	chooser_image = gtk_file_chooser_button_new("Select image", GTK_FILE_CHOOSER_ACTION_OPEN);
	
	/* set image chooser's filters */
	GtkFileFilter *filter_all, *supported[5];	
	filter_all = gtk_file_filter_new();
	gtk_file_filter_set_name(filter_all,"All supported images");
	
	supported[0] = gtk_file_filter_new();
	gtk_file_filter_set_name(supported[0],"BMP");
	gtk_file_filter_add_mime_type(supported[0],"image/bmp");
	gtk_file_filter_add_mime_type(filter_all,"image/bmp");
	
	supported[1] = gtk_file_filter_new();
	gtk_file_filter_set_name(supported[1],"JPEG");
	gtk_file_filter_add_mime_type(supported[1],"image/jpeg");
	gtk_file_filter_add_mime_type(filter_all,"image/jpeg");
	
	supported[2] = gtk_file_filter_new();
	gtk_file_filter_set_name(supported[2],"GIF");
	gtk_file_filter_add_mime_type(supported[2],"image/gif");
	gtk_file_filter_add_mime_type(filter_all,"image/gif");
	
	supported[3] = gtk_file_filter_new();
	gtk_file_filter_set_name(supported[3],"PNG");
	gtk_file_filter_add_mime_type(supported[3],"image/png");
	gtk_file_filter_add_mime_type(filter_all,"image/png");
	
	supported[4] = gtk_file_filter_new();
	gtk_file_filter_set_name(supported[4],"TIFF");
	gtk_file_filter_add_mime_type(supported[4],"image/tiff");
	gtk_file_filter_add_mime_type(filter_all,"image/tiff");
	
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser_image), filter_all);
	int i;
	for(i = 0; i < 5; i++) {
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser_image), supported[i]);
	}
	
	if (settings->imagefile != NULL) {
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(chooser_image), settings->imagefile);
	}
	gtk_widget_set_size_request (chooser_image, INPUT_W, INPUT_H);
	
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
	
	frame_position = gtk_frame_new("Position on the image:");
	gtk_widget_set_size_request (frame_position, FRAME_POSITION_W, FRAME_POSITION_H);
	table_position = gtk_table_new(3, 3, TRUE);
	
	button_tl = gtk_radio_button_new (NULL);
	gtk_button_set_image(GTK_BUTTON(button_tl), gtk_image_new_from_pixbuf(gdk_pixbuf_from_pixdata(&pixdata_postl, TRUE, NULL)));
	gtk_widget_set_tooltip_text (button_tl, watermark_position_string[WM_POS_TL]);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button_tl), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_tl), settings->position == WM_POS_TL);
	gtk_widget_set_size_request (button_tl, BUTTON_POSITION_W, BUTTON_POSITION_H);
	gtk_table_attach(GTK_TABLE(table_position), button_tl, 0, 1, 0, 1, GTK_EXPAND, GTK_EXPAND, 0, 0);
	
	button_tr = gtk_radio_button_new_from_widget (GTK_RADIO_BUTTON(button_tl));
	gtk_button_set_image(GTK_BUTTON(button_tr), gtk_image_new_from_pixbuf(gdk_pixbuf_from_pixdata(&pixdata_postr, TRUE, NULL)));
	gtk_widget_set_tooltip_text (button_tr, watermark_position_string[WM_POS_TR]);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button_tr), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_tr), settings->position == WM_POS_TR);
	gtk_widget_set_size_request (button_tr, BUTTON_POSITION_W, BUTTON_POSITION_H);
	gtk_table_attach(GTK_TABLE(table_position), button_tr, 2, 3, 0, 1, GTK_EXPAND, GTK_EXPAND, 0, 0);
	
	button_cc = gtk_radio_button_new_from_widget (GTK_RADIO_BUTTON(button_tl));
	gtk_button_set_image(GTK_BUTTON(button_cc), gtk_image_new_from_pixbuf(gdk_pixbuf_from_pixdata(&pixdata_poscc, TRUE, NULL)));
	gtk_widget_set_tooltip_text (button_cc, watermark_position_string[WM_POS_CC]);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button_cc), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_cc), settings->position == WM_POS_CC);
	gtk_widget_set_size_request (button_cc, BUTTON_POSITION_W, BUTTON_POSITION_H);
	gtk_table_attach(GTK_TABLE(table_position), button_cc, 1, 2, 1, 2, GTK_EXPAND, GTK_EXPAND, 0, 0);
	
	button_bl = gtk_radio_button_new_from_widget (GTK_RADIO_BUTTON(button_tl));
	gtk_button_set_image(GTK_BUTTON(button_bl), gtk_image_new_from_pixbuf(gdk_pixbuf_from_pixdata(&pixdata_posbl, TRUE, NULL)));
	gtk_widget_set_tooltip_text (button_bl, watermark_position_string[WM_POS_BL]);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button_bl), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_bl), settings->position == WM_POS_BL);
	gtk_widget_set_size_request (button_bl, BUTTON_POSITION_W, BUTTON_POSITION_H);
	gtk_table_attach(GTK_TABLE(table_position), button_bl, 0, 1, 2, 3, GTK_EXPAND, GTK_EXPAND, 0, 0);
	
	button_br = gtk_radio_button_new_from_widget (GTK_RADIO_BUTTON(button_tl));
	gtk_button_set_image(GTK_BUTTON(button_br), gtk_image_new_from_pixbuf(gdk_pixbuf_from_pixdata(&pixdata_posbr, TRUE, NULL)));
	gtk_widget_set_tooltip_text (button_br, watermark_position_string[WM_POS_BR]);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button_br), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_br), settings->position == WM_POS_BR);
	gtk_widget_set_size_request (button_br, BUTTON_POSITION_W, BUTTON_POSITION_H);
	gtk_table_attach(GTK_TABLE(table_position), button_br, 2, 3, 2, 3, GTK_EXPAND, GTK_EXPAND, 0, 0);
	
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
	
	gtk_container_add(GTK_CONTAINER(frame_position), table_position);
	gtk_box_pack_start(GTK_BOX(gui), frame_position, FALSE, FALSE, 0);
	
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
	
	orig_settings->text = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_text)));
		
	char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser_image));
	if (filename != NULL) {
		orig_settings->imagefile = g_strdup(filename);
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
