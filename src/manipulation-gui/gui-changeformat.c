#include <stdlib.h>
#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include "gui-changeformat.h"
#include "../bimp-manipulations.h"

static void update_frame_params(GtkComboBox*, changeformat_settings);

GtkWidget *frame_params, *inner_widget;
GtkWidget *combo_format, *scale_quality, *check_interlace, *scale_compression, *check_rle, *combo_compression;

GtkWidget* bimp_changeformat_gui_new(changeformat_settings settings)
{
	GtkWidget *gui;
	
	gui = gtk_vbox_new(FALSE, 5);
	
	combo_format = gtk_combo_box_new_text();
	gtk_widget_set_size_request (combo_format, COMBO_FORMAT_W, COMBO_FORMAT_H);
	int i;
	for(i = 0; i < FORMAT_END; i++) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo_format), format_type_string[i][1]);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_format), settings->format);
	
	frame_params = gtk_frame_new("Format settings:");
	gtk_widget_set_size_request (frame_params, FRAME_PARAMS_W, FRAME_PARAMS_H);
	
	gtk_box_pack_start(GTK_BOX(gui), combo_format, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), frame_params, FALSE, FALSE, 0);
	
	update_frame_params(GTK_COMBO_BOX(combo_format), settings);	
	
	g_signal_connect(G_OBJECT(combo_format), "changed", G_CALLBACK(update_frame_params), settings);
	
	return gui;
}

static void update_frame_params(GtkComboBox *widget, changeformat_settings settings) 
{
	format_type selected_format = (format_type)gtk_combo_box_get_active(widget);
	
	if (selected_format == FORMAT_GIF) {		
		inner_widget = gtk_vbox_new(FALSE, 5);
		gtk_container_set_border_width(GTK_CONTAINER(inner_widget), 8);
		check_interlace = gtk_check_button_new_with_label("Interlaced");
		
		if (selected_format == settings->format) {
			format_params_gif settings_gif = (format_params_gif)(settings->params);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_interlace), settings_gif->interlace);
		}
		else {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_interlace), FALSE);
		}
		
		gtk_box_pack_start(GTK_BOX(inner_widget), check_interlace, FALSE, FALSE, 0);
	} 
	else if (selected_format == FORMAT_JPEG) {
		GtkWidget *hbox_quality, *label_quality;
		
		inner_widget = gtk_vbox_new(FALSE, 5);
		gtk_container_set_border_width(GTK_CONTAINER(inner_widget), 8);
		hbox_quality = gtk_hbox_new(FALSE, 5);
		label_quality = gtk_label_new("Quality");
		gtk_widget_set_size_request (label_quality, LABEL_QUALITY_W, LABEL_QUALITY_H);
		gtk_misc_set_alignment(GTK_MISC(label_quality), 0.5, 0.8);
		scale_quality = gtk_hscale_new_with_range(0, 100, 1);
		gtk_widget_set_size_request (scale_quality, SCALE_QUALITY_W, SCALE_QUALITY_H);
		
		if (selected_format == settings->format) {
			format_params_jpeg settings_jpeg = (format_params_jpeg)(settings->params);
			gtk_range_set_value(GTK_RANGE(scale_quality), settings_jpeg->quality);
		}
		else {
			gtk_range_set_value(GTK_RANGE(scale_quality), 85.0);
		}
		
		gtk_box_pack_start(GTK_BOX(hbox_quality), label_quality, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox_quality), scale_quality, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(inner_widget), hbox_quality, FALSE, FALSE, 0);
	}
	else if (selected_format == FORMAT_PNG) {
		GtkWidget *hbox_compression, *label_compression;
		
		inner_widget = gtk_vbox_new(FALSE, 5);
		gtk_container_set_border_width(GTK_CONTAINER(inner_widget), 8);
		check_interlace = gtk_check_button_new_with_label("Interlace (Adam7)");
		hbox_compression = gtk_hbox_new(FALSE, 5);
		label_compression = gtk_label_new("Compression");
		gtk_widget_set_size_request (label_compression, LABEL_COMPRESSION_W, LABEL_COMPRESSION_H);
		gtk_misc_set_alignment(GTK_MISC(label_compression), 0.5, 0.8);
		scale_compression = gtk_hscale_new_with_range(0, 9, 1);
		gtk_widget_set_size_request (scale_compression, SCALE_COMPRESSION_W, SCALE_COMPRESSION_H);
		
		if (selected_format == settings->format) {
			format_params_png settings_png = (format_params_png)(settings->params);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_interlace), settings_png->interlace);
			gtk_range_set_value(GTK_RANGE(scale_compression), settings_png->compression);
		}
		else {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_interlace), FALSE);
			gtk_range_set_value(GTK_RANGE(scale_compression), 9);
		}
		
		gtk_box_pack_start(GTK_BOX(inner_widget), check_interlace, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox_compression), label_compression, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox_compression), scale_compression, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(inner_widget), hbox_compression, FALSE, FALSE, 0);
	}
	else if (selected_format == FORMAT_TGA) {		
		inner_widget = gtk_vbox_new(FALSE, 5);
		gtk_container_set_border_width(GTK_CONTAINER(inner_widget), 8);
		check_rle = gtk_check_button_new_with_label("RLE compression");
		
		if (selected_format == settings->format) {
			format_params_tga settings_tga = (format_params_tga)(settings->params);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_rle), settings_tga->rle);
		}
		else {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_rle), FALSE);
		}
		
		gtk_box_pack_start(GTK_BOX(inner_widget), check_rle, FALSE, FALSE, 0);
	}
	else if (selected_format == FORMAT_TIFF) {
		GtkWidget *hbox_compression, *label_compression;
		
		inner_widget = gtk_vbox_new(FALSE, 5);
		gtk_container_set_border_width(GTK_CONTAINER(inner_widget), 8);
		hbox_compression = gtk_hbox_new(FALSE, 5);
		label_compression = gtk_label_new("Compression");
		gtk_widget_set_size_request (label_compression, LABEL_COMPRESSION_W, LABEL_COMPRESSION_H);
		gtk_misc_set_alignment(GTK_MISC(label_compression), 0.5, 0.5);
		combo_compression = gtk_combo_box_new_text();
		gtk_widget_set_size_request (combo_compression, COMBO_COMPRESSION_W, COMBO_COMPRESSION_H);
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo_compression), "None");
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo_compression), "LZW");
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo_compression), "Pack bits");
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo_compression), "Deflate");
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo_compression), "JPEG");
		
		if (selected_format == settings->format) {
			format_params_tiff settings_tiff = (format_params_tiff)(settings->params);
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo_compression), settings_tiff->compression);
		}
		else {
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo_compression), 0);
		}
		
		gtk_box_pack_start(GTK_BOX(hbox_compression), label_compression, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox_compression), combo_compression, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(inner_widget), hbox_compression, FALSE, FALSE, 0);
	}
	else {
		inner_widget = gtk_label_new("This format has no params");
	}
	
	if (gtk_bin_get_child(GTK_BIN(frame_params)) != NULL) {
		gtk_widget_destroy(gtk_bin_get_child(GTK_BIN(frame_params)));
	}
	gtk_container_add(GTK_CONTAINER(frame_params), inner_widget);
	gtk_widget_show_all(frame_params);
}

void bimp_changeformat_save(changeformat_settings orig_settings) 
{
	orig_settings->format = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_format));
	free(orig_settings->params);
	
	if (orig_settings->format == FORMAT_GIF) {
		orig_settings->params = (format_params_gif) g_malloc(sizeof(struct changeformat_params_gif));
		((format_params_gif)orig_settings->params)->interlace = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_interlace));
	}
	else if (orig_settings->format == FORMAT_JPEG) {
		orig_settings->params = (format_params_jpeg) g_malloc(sizeof(struct changeformat_params_jpeg));
		((format_params_jpeg)orig_settings->params)->quality = gtk_range_get_value(GTK_RANGE(scale_quality));
	}
	else if (orig_settings->format == FORMAT_PNG) {
		orig_settings->params = (format_params_png) g_malloc(sizeof(struct changeformat_params_png));
		((format_params_png)orig_settings->params)->interlace = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_interlace));
		((format_params_png)orig_settings->params)->compression = gtk_range_get_value(GTK_RANGE(scale_compression));
	}
	else if (orig_settings->format == FORMAT_TGA) {
		orig_settings->params = (format_params_tga) g_malloc(sizeof(struct changeformat_params_tga));
		((format_params_tga)orig_settings->params)->rle = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_rle));
	}
	else if (orig_settings->format == FORMAT_TIFF) {
		orig_settings->params = (format_params_png) g_malloc(sizeof(struct changeformat_params_png));
		((format_params_tiff)orig_settings->params)->compression = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_compression));
	}
	else {
		orig_settings->params = NULL;
	}
}



