/*
 * Functions called when the user clicks on 'APPLY'
 */

#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <libgimpbase/gimpbase.h>
#include <glib.h>
#include <glib/gstdio.h>
#include "bimp-operate.h"
#include "bimp.h"
#include "bimp-manipulations.h"
#include "bimp-gui.h"
#include "bimp-utils.h"
#include "plugin-intl.h"

static gboolean process_image(gpointer);
static gboolean apply_manipulation(manipulation, image_output);
static gboolean apply_resize(resize_settings, image_output);
static gboolean apply_crop(crop_settings, image_output);
static gboolean apply_fliprotate(fliprotate_settings, image_output);
static gboolean apply_color(color_settings, image_output);
static gboolean apply_sharpblur(sharpblur_settings, image_output);
static gboolean apply_watermark(watermark_settings, image_output);
static gboolean apply_userdef(userdef_settings, image_output);
static gboolean apply_rename(rename_settings, image_output, char*);
static gboolean image_save(format_type, image_output, format_params);
static gboolean image_save_bmp(image_output);
static gboolean image_save_gif(image_output, gboolean);
static gboolean image_save_icon(image_output);
static gboolean image_save_jpeg(image_output, float, float, gboolean, gboolean, gchar*, int, gboolean, int, int);
static gboolean image_save_png(image_output, gboolean, int, gboolean, gboolean, gboolean, gboolean, gboolean, gboolean, gboolean);
static gboolean image_save_tga(image_output, gboolean, int);
static gboolean image_save_tiff(image_output, int);
static gchar** array_path_folders (char *path);
static int overwrite_result(char*, GtkWidget*);
static char* get_datetime(void);

char* current_datetime;
int processed_count;
int total_images;

char* common_folder_path;

void bimp_start_batch(gpointer parent_dialog)
{
	bimp_set_busy(TRUE);
	
	g_print("\nBIMP - Batch Manipulation Plugin\nStart batch processing...\n");
	processed_count = 0;
	total_images = g_slist_length(bimp_input_filenames);
	
	bimp_progress_bar_set(0.0, "");
	
	current_datetime = get_datetime();
	common_folder_path = NULL;

	if (bimp_keepfolderhierarchy){
		int i, j;
		gboolean need_hierarchy = FALSE;
		char * path = NULL;
		char ** common_folder;
		char ** current_folder;
		size_t common_folder_size, current_folder_size;
		
		path = bimp_comp_get_filefolder(g_slist_nth(bimp_input_filenames,0)->data);
		
		common_folder = array_path_folders(path);
		common_folder_size = 0;
		for (common_folder_size = 0; common_folder[common_folder_size]!= NULL;++common_folder_size);

		for (i=1; i < total_images ; i++)
		{
			path = bimp_comp_get_filefolder(g_slist_nth(bimp_input_filenames,i)->data);
			current_folder = array_path_folders (path);
			for (current_folder_size = 0; current_folder[current_folder_size]!= NULL;++current_folder_size);

			// The common path is at most as long as the shortest path
			while (common_folder_size > current_folder_size)
			{
				need_hierarchy = TRUE;
				g_free(common_folder[common_folder_size-1]);
				common_folder[common_folder_size-1] = NULL;
				common_folder_size--;
			}
			
			for (j=0; j < common_folder_size; ++j)
			{
				if (strcmp(common_folder[j], current_folder[j]) != 0) {
					need_hierarchy = TRUE;
					while (common_folder_size > j)
					{
						g_free(common_folder[common_folder_size-1]);
						common_folder[common_folder_size-1] = NULL;
						common_folder_size--;
					}
					break;
				}
			}

			g_strfreev(current_folder);
		}

		if (need_hierarchy) 
			common_folder_path = g_strjoinv(FILE_SEPARATOR_STR,common_folder);
		
		g_strfreev(common_folder);
	}

	guint batch_idle_tag = g_idle_add((GSourceFunc)process_image,parent_dialog);
}

static gchar** array_path_folders (char *path)
{
	GArray *array = NULL;
	int i; 
	char * normalized_path = (char*)g_malloc(sizeof(path));

	normalized_path = g_strdup(path);
	return g_strsplit(normalized_path,FILE_SEPARATOR_STR,0);
}

static gboolean process_image(gpointer parent)
{
	image_output imageout = (image_output)g_malloc(sizeof(struct imageout_str));
	char* orig_filename = NULL;
	char* orig_basename = NULL;
	char* orig_file_ext = NULL;
	char* output_file_comp = NULL;
	
	/* load the file and assign ids */
	orig_filename = g_slist_nth (bimp_input_filenames, processed_count)->data;
	
	orig_basename = g_strdup(bimp_comp_get_filename(orig_filename)); /* store original filename */
	orig_file_ext = g_strdup(strrchr(orig_basename, '.')); /* store original extension */
	
	g_print("\nWorking on file #%d: %s (%s)\n", processed_count+1, orig_basename, orig_filename);
	bimp_progress_bar_set(((double)processed_count)/total_images, g_strdup_printf(_("Working on file %s..."), orig_filename));
	
	bimp_apply_drawable_manipulations(imageout, (gchar*)orig_filename, (gchar*)orig_basename); 

	/* rename and save process... */
	orig_basename[strlen(orig_basename) - strlen(orig_file_ext)] = '\0'; /* remove extension from basename */
	
	/* check if a rename pattern is defined */
	if(bimp_list_contains_manip(MANIP_RENAME)) {
		g_print("Applying RENAME...\n");
		apply_rename((rename_settings)(bimp_list_get_manip(MANIP_RENAME))->settings, imageout, orig_basename);
	}
	else {
		imageout->filename = orig_basename;
	}

	/* To keep the folder hierarchy */
	if (common_folder_path == NULL)
	{
		// Not selected or required, everything goes into the same destination folder
		output_file_comp = "";
	}
	else
	{
		// keep folders to add to output path
		output_file_comp = 
			g_strndup(&orig_filename[strlen(common_folder_path)+1],
			strlen(orig_filename)-(strlen(common_folder_path)+1)
			-strlen(orig_basename)-strlen(orig_file_ext)); 
	}
	
	if (strlen(output_file_comp) > 0)
	{
#ifdef _WIN32		
		// Clean output_file_comp
		// Should only be concerned for ':' in Drive letter
		for (i = 0; i < strlen(output_file_comp); ++i)
			if ( output_file_comp[i] == ':' )
				output_file_comp[i] = '_';
#endif
		// Create path if needed
		g_mkdir_with_parents(g_strconcat(bimp_output_folder, 
										 FILE_SEPARATOR_STR,
										 output_file_comp, NULL), 0777);
	}
	
	/* save the final image in output dir with proper format */
	if(bimp_list_contains_manip(MANIP_CHANGEFORMAT)) {
		changeformat_settings settings = (changeformat_settings)(bimp_list_get_manip(MANIP_CHANGEFORMAT))->settings;
		format_type type = settings->format;
		format_params params = settings->params;
		
		g_print("Changing FORMAT to %s\n", format_type_string[type][0]);
		imageout->filename = g_strconcat(imageout->filename, ".", format_type_string[type][0], NULL); /* append new file extension */
		
		imageout->filepath = g_strconcat(bimp_output_folder, FILE_SEPARATOR_STR, output_file_comp, imageout->filename, NULL); /* build new path */
		g_print("Saving file %s in %s\n", imageout->filename, imageout->filepath);
		
		int ow_res = overwrite_result(imageout->filepath, parent);
		if (ow_res > 0) {
			image_save(type, imageout, params);
			
			/* TODO?
			if (ow_res == 2 && bimp_deleteondone) {
				if (g_remove (orig_filename) < 0) 
					g_print("Error, Can't remove old file!\n");
				}
				else obsolete_file[processed_count] = TRUE;
			}*/
		}
		else {
			g_print("File has not been overwritten!\n");
		}
	}
	else {
		/* if not specified, save in original format */
		imageout->filename = g_strconcat(imageout->filename, orig_file_ext, NULL); /* append old file extension */
		imageout->filepath = g_strconcat(bimp_output_folder, FILE_SEPARATOR_STR, output_file_comp, imageout->filename, NULL); /* build new path */
		
		int ow_res = overwrite_result(imageout->filepath, parent);
		if (ow_res > 0) {
			g_print("Saving file %s in %s\n", imageout->filename, imageout->filepath);
			image_save(-1, imageout, NULL);
			
			/*
			if (ow_res == 2 && bimp_deleteondone) {
				if (g_remove (orig_filename) < 0) 
					g_print("Error, Can't remove old file!\n");
				}
				else obsolete_file[processed_count] = TRUE;
			}*/
		}
		else {
			g_print("File has not been overwritten!\n");
		}
	}

	gimp_image_delete(imageout->image_id); /* is it useful? */
	g_free(orig_basename);
	g_free(orig_file_ext);
	g_free(output_file_comp);
	g_free(imageout);

	processed_count++;
	
	/* TODO: errors check here */
	if (!bimp_is_busy) {
		bimp_progress_bar_set(0.0, _("Operations stopped"));
		g_print("\nStopped, %d files processed.\n", processed_count);
		return FALSE;
	}
	else {
		if (processed_count == total_images) {
			bimp_progress_bar_set(1.0, _("End, all files have been processed"));
			g_print("\nEnd, all %d files have been processed.\n", processed_count);
			
			/*int i;
			for (i = 0; i < total_images; i++) {
				if (obsolete_file[i]) bimp_input_filenames = g_slist_delete_link(bimp_input_filenames, g_slist_nth (bimp_input_filenames, i));
			}
			bimp_refresh_fileview();
			*/
			bimp_set_busy(FALSE);
			
			return FALSE;
		}
		else {
			return TRUE;
		}
	}
}

void bimp_apply_drawable_manipulations(image_output imageout, gchar* orig_filename, gchar* orig_basename)
{
	imageout->image_id = gimp_file_load(GIMP_RUN_NONINTERACTIVE, orig_filename, orig_basename); /* load file and get image id */
	/* LOAD ERROR CHECK HERE */
	g_print("Image ID is %d\n", imageout->image_id);
	
	imageout->drawable_id = gimp_image_merge_visible_layers(imageout->image_id, GIMP_CLIP_TO_IMAGE); /* merge levels and get drawable id */
	gimp_layer_add_alpha (imageout->drawable_id);
	g_print("Drawable ID is %d\n", imageout->drawable_id);
	
	/* start manipulations sequence, resize and crop start first */
	if (bimp_list_contains_manip(MANIP_RESIZE)) {
		g_print("Applying RESIZE...\n");
		apply_resize((resize_settings)(bimp_list_get_manip(MANIP_RESIZE))->settings, imageout);
	}
	
	if (bimp_list_contains_manip(MANIP_CROP)) {
		g_print("Applying CROP...\n");
		apply_crop((crop_settings)(bimp_list_get_manip(MANIP_CROP))->settings, imageout);
	}
	
	/* apply all the intermediate manipulations */
	g_slist_foreach(bimp_selected_manipulations, (GFunc)apply_manipulation, imageout);
	
	/*  watermark at last */
	if(bimp_list_contains_manip(MANIP_WATERMARK)) {
		g_print("Applying WATERMARK...\n");
		apply_watermark((watermark_settings)(bimp_list_get_manip(MANIP_WATERMARK))->settings, imageout);
	}	
}

static gboolean apply_manipulation(manipulation man, image_output out) 
{
	gboolean success = TRUE;
	
	if (man->type == MANIP_FLIPROTATE) {
		g_print("Applying FLIP OR ROTATE...\n");
		success = apply_fliprotate((fliprotate_settings)(man->settings), out);
	}
	else if (man->type == MANIP_COLOR) {
		g_print("Applying COLOR CORRECTION...\n");
		success = apply_color((color_settings)(man->settings), out);
	}
	else if (man->type == MANIP_SHARPBLUR) {
		g_print("Applying SHARPBLUR...\n");
		success = apply_sharpblur((sharpblur_settings)(man->settings), out);
	}
	else if (man->type == MANIP_USERDEF) {
		g_print("Applying %s...\n", ((userdef_settings)(man->settings))->procedure);
		success = apply_userdef((userdef_settings)(man->settings), out);
	}
	
	return success;
}

static gboolean apply_resize(resize_settings settings, image_output out) 
{
	gboolean success = FALSE;
	gint final_w, final_h;
	gdouble orig_res_x, orig_res_y;
	
	if (settings->change_res) {
		success = gimp_image_get_resolution(
			out->image_id,
			&orig_res_x,
			&orig_res_y
		);
		
		if ((settings->new_res_x != orig_res_x) || (settings->new_res_y != orig_res_y)) {
			/* change resolution */
			success = gimp_image_set_resolution(
				out->image_id,
				settings->new_res_x,
				settings->new_res_y
			);
		}
	}
	
	
	if (settings->resize_mode == RESIZE_PERCENT) {
		/* user selected a percentage of the original size */
		final_w = round((gimp_image_width(out->image_id) * settings->new_w_pc) / 100.0);
		final_h = round((gimp_image_height(out->image_id) * settings->new_h_pc) / 100.0);
	}
	else {
		/* user typed exact pixel size */
		if (settings->resize_mode == RESIZE_PIXEL_WIDTH) {
			/* width only */
			final_w = settings->new_w_px;
			if (settings->aspect_ratio) {
				/* and maintain aspect ratio */
				final_h = round(((float)settings->new_w_px * gimp_image_height(out->image_id)) / gimp_image_width(out->image_id));
			}
			else {
				final_h = gimp_image_height(out->image_id);
			}
		}
		else if (settings->resize_mode == RESIZE_PIXEL_HEIGHT) {
			/* height only */
			final_h = settings->new_h_px;
			if (settings->aspect_ratio) {
				/* and maintain aspect ratio */
				final_w = round(((float)settings->new_h_px * gimp_image_width(out->image_id)) / gimp_image_height(out->image_id));
			}
			else {
				final_w = gimp_image_width(out->image_id);
			}
		}
		else {
			/* both dimensions are defined */
			final_w = settings->new_w_px;
			final_h = settings->new_h_px;
		}
	}
	
	/* do resize */
	if (GIMP_MAJOR_VERSION == 2 && GIMP_MINOR_VERSION <= 6) {
		success = gimp_image_scale_full (
			out->image_id, 
			final_w, 
			final_h, 
			settings->interpolation
		);
	}
	else {
	/* starting from 2.8, gimp_image_scale_full is deprecated. 
	 * use gimp_image_scale instead */
		GimpInterpolationType oldInterpolation;
		oldInterpolation = gimp_context_get_interpolation();
		
		success = gimp_context_set_interpolation (settings->interpolation);
		
		success = gimp_image_scale (
			out->image_id, 
			final_w, 
			final_h
		);
		
		success = gimp_context_set_interpolation (oldInterpolation);
	}
	
	return success;
}

static gboolean apply_crop(crop_settings settings, image_output out) 
{
	gboolean success = TRUE;
	gint newWidth, newHeight, oldWidth, oldHeight, posX, posY;
	
	oldWidth = gimp_image_width(out->image_id);
	oldHeight = gimp_image_height(out->image_id);
	
	if (settings->manual) {
		newWidth = settings->new_w;
		newHeight = settings->new_h;
		posX = (oldWidth - newWidth) / 2;
		posY = (oldHeight - newHeight) / 2;
	}
	else {
		float ratio1, ratio2;
		if (settings->ratio == CROP_PRESET_CUSTOM) {
			ratio1 = settings->custom_ratio1;
			ratio2 = settings->custom_ratio2;
		}
		else {
			ratio1 = (float)crop_preset_ratio[settings->ratio][0];
			ratio2 = (float)crop_preset_ratio[settings->ratio][1];
		}
		
		if (( (float)oldWidth / oldHeight ) > ( ratio1 / ratio2) ) { 
			/* crop along the width */
			newHeight = oldHeight;
			newWidth = round(( ratio1 * (float)newHeight ) / ratio2);
			posX = (oldWidth - newWidth) / 2;
			posY = 0;
		} else { 
			/* crop along the height */
			newWidth = oldWidth;
			newHeight = round(( ratio2 * (float)newWidth) / ratio1);
			posX = 0;
			posY = (oldHeight - newHeight) / 2;
		}
	}
	
	success = gimp_image_crop (
		out->image_id,
		newWidth,
		newHeight,
		posX,
		posY
	);
	
	return success;
}

static gboolean apply_fliprotate(fliprotate_settings settings, image_output out) 
{
	gboolean success = TRUE;
	
	if (settings->flip_h) {
		/* do horizontal flip */
		success = gimp_image_flip (
			out->image_id,
			GIMP_ORIENTATION_HORIZONTAL
		);
	}
	
	if (settings->flip_v) {
		/* do vertical flip */
		success = gimp_image_flip (
			out->image_id,
			GIMP_ORIENTATION_VERTICAL
		);
	}
	
	if (settings->rotate) {
		/* do rotation */
		success = gimp_image_rotate (
			out->image_id,
			settings->rotation_type
		);
	}
	
	return success;
}

static gboolean apply_color(color_settings settings, image_output out) 
{
	gboolean success = TRUE;
	
	if (settings->brightness != 0 || settings->contrast != 0) {
		/* brightness or contrast have been modified, apply the manipulation */
		success = gimp_brightness_contrast(
			out->drawable_id, 
			settings->brightness, 
			settings->contrast
		);
	}
	
	if (settings->grayscale && !gimp_drawable_is_gray(out->drawable_id)) {
		/* do grayscale conversion */
		success = gimp_image_convert_grayscale(out->image_id);
	}
	
	if (settings->levels_auto) {
		/* do levels correction */
		success = gimp_levels_stretch(out->drawable_id);
	}
	
	return success;
}

static gboolean apply_sharpblur(sharpblur_settings settings, image_output out) 
{
	gboolean success = TRUE;
	gint nreturn_vals;
	
	if (settings->amount < 0) {
		/* do sharp */
		GimpParam *return_vals = gimp_run_procedure(
			"plug_in_sharpen", /* could use plug_in_unsharp_mask, but there's a datatype bug in 2.6.x version */
			&nreturn_vals,
			GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
			GIMP_PDB_IMAGE, out->image_id,
			GIMP_PDB_DRAWABLE, out->drawable_id,
			GIMP_PDB_INT32, -(settings->amount),
			GIMP_PDB_END
		);
	} else if (settings->amount > 0){
		/* do blur */
		float minsize = bimp_min(gimp_image_width(out->image_id)/4, gimp_image_height(out->image_id)/4);
		float radius = (minsize / 100) * settings->amount;
		GimpParam *return_vals = gimp_run_procedure(
			"plug_in_gauss",
			&nreturn_vals,
			GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
			GIMP_PDB_IMAGE, out->image_id,
			GIMP_PDB_DRAWABLE, out->drawable_id,
			GIMP_PDB_FLOAT, radius,
			GIMP_PDB_FLOAT, radius,
			GIMP_PDB_INT32, 0,
			GIMP_PDB_END
		);
	}
	
	return success;
}

static gboolean apply_watermark(watermark_settings settings, image_output out) 
{
	gboolean success = TRUE;
	gint32 layerId;
	gdouble posX, posY;
	gint wmwidth, wmheight, wmasc, wmdesc;
	
	if (settings->mode) {
		if (strlen(settings->text) == 0) {
			return TRUE;
		}
		
		GimpRGB old_foreground, new_foreground;
		
		gimp_context_get_foreground(&old_foreground);
		gimp_rgb_parse_hex (&new_foreground, gdk_color_to_string(&(settings->color)), strlen(gdk_color_to_string(&(settings->color))));
		gimp_context_set_foreground(&new_foreground);
		
		gimp_text_get_extents_fontname(
			settings->text,
			pango_font_description_get_size(settings->font) / PANGO_SCALE,
			GIMP_PIXELS,
			pango_font_description_get_family(settings->font),
			&wmwidth,
			&wmheight,
			&wmasc,
			&wmdesc
		);

		if (settings->position == WM_POS_TL) {
			posX = 10;
			posY = 5;
		}
		else if (settings->position == WM_POS_TR) {
			posX = gimp_image_width(out->image_id) - wmwidth - 10;
			posY = 5;
		}
		else if (settings->position == WM_POS_BL) {
			posX = 10;
			posY = gimp_image_height(out->image_id) - wmheight - 5;
		}
		else if (settings->position == WM_POS_BR) {
			posX = gimp_image_width(out->image_id) - wmwidth - 10;
			posY = gimp_image_height(out->image_id) - wmheight - 5;
		}
		else {
			posX = (gimp_image_width(out->image_id) / 2) - (wmwidth / 2);
			posY = (gimp_image_height(out->image_id) / 2) - (wmheight / 2);
		}
		
		layerId = gimp_text_fontname(
			out->image_id,
			-1,
			posX,
			posY,
			settings->text,
			-1,
			TRUE,
			pango_font_description_get_size(settings->font) / PANGO_SCALE,
			GIMP_PIXELS,
			pango_font_description_get_family(settings->font)
		);
		gimp_context_set_foreground(&old_foreground);
		gimp_layer_set_opacity(layerId, settings->opacity);
	}
	else {
		if (!g_file_test(settings->image_file, G_FILE_TEST_IS_REGULAR)) {//((access(settings->image_file, R_OK) == -1)) {
			// error, can't access image file
			return TRUE;
		}
		
		layerId = gimp_file_load_layer(
			GIMP_RUN_NONINTERACTIVE,
			out->image_id,
			settings->image_file
		);
		
		gimp_layer_set_opacity(layerId, settings->opacity);
		wmwidth = gimp_drawable_width(layerId);
		wmheight = gimp_drawable_height(layerId);
		
		if (GIMP_MAJOR_VERSION == 2 && GIMP_MINOR_VERSION <= 6) {
		// #if (GIMP_MAJOR_VERSION == 2) && (GIMP_MINOR_VERSION <= 6)
			gimp_image_add_layer(
				out->image_id,
				layerId,
				0
			);
		}
		else {
		//#else
		/* starting from 2.8, gimp_image_add_layer is deprecated. 
		 * use gimp_image_insert_layer instead */
			gimp_image_insert_layer(
				out->image_id,
				layerId,
				0,
				0
			);
		}
		//#endif
		if (settings->position == WM_POS_TL) {
			posX = 10;
			posY = 10;
		}
		else if (settings->position == WM_POS_TR) {
			posX = gimp_image_width(out->image_id) - wmwidth - 10;
			posY = 10;
		}
		else if (settings->position == WM_POS_BL) {
			posX = 10;
			posY = gimp_image_height(out->image_id) - wmheight - 10;
		}
		else if (settings->position == WM_POS_BR) {
			posX = gimp_image_width(out->image_id) - wmwidth - 10;
			posY = gimp_image_height(out->image_id) - wmheight - 10;
		}
		else {
			posX = (gimp_image_width(out->image_id) / 2) - (wmwidth / 2);
			posY = (gimp_image_height(out->image_id) / 2) - (wmheight / 2);
		}
		
		gimp_layer_set_offsets(
			layerId,
			posX,
			posY
		);   
	}
	
	out->drawable_id = gimp_image_merge_visible_layers(out->image_id, GIMP_CLIP_TO_IMAGE);
	
	return success;
}

static gboolean apply_userdef(userdef_settings settings, image_output out) 
{
	gboolean success = TRUE;
	
	int param_i;
	for (param_i = 0; param_i < settings->num_params; param_i++) {
		switch((settings->params[param_i]).type) {
			case GIMP_PDB_IMAGE: 
				(settings->params[param_i]).data.d_image = out->image_id;
				break;
			
			case GIMP_PDB_DRAWABLE:
				(settings->params[param_i]).data.d_drawable = out->drawable_id;
				break;
			
			default: break;
		}
	}
	
	gint nreturn_vals;
	GimpParam *return_vals = gimp_run_procedure2(
		settings->procedure,
		&nreturn_vals,
		settings->num_params,
		settings->params
	);
	
	out->drawable_id = gimp_image_merge_visible_layers(out->image_id, GIMP_CLIP_TO_IMAGE);
	
	return success;
}

static gboolean apply_rename(rename_settings settings, image_output out, char* orig_basename) 
{
	char *orig_name = g_strdup(orig_basename);
	
	out->filename = g_strdup(settings->pattern);
	
	/* search for 'RENAME_KEY_ORIG' occurrences and replace the final filename */
	if(strstr(out->filename, RENAME_KEY_ORIG) != NULL) {
		out->filename = bimp_str_replace(out->filename, RENAME_KEY_ORIG, orig_name);
	}
	
	/* same thing for count and datetime */
	
	if(strstr(out->filename, RENAME_KEY_COUNT) != NULL)	{
		char strcount[5];
		sprintf(strcount, "%i", processed_count + 1);
		out->filename = bimp_str_replace(out->filename, RENAME_KEY_COUNT, strcount);
	}
	
	if(strstr(out->filename, RENAME_KEY_DATETIME) != NULL)	{
		out->filename = bimp_str_replace(out->filename, RENAME_KEY_DATETIME, current_datetime);
	}
	
	g_free(orig_name);
	
	return TRUE;
}

/* following: set of functions that saves the image file in various formats */

static gboolean image_save(format_type type, image_output imageout, format_params params) 
{
	gboolean result;
	
	if (type == FORMAT_BMP) {
		result = image_save_bmp(imageout);
	}
	else if(type == FORMAT_GIF) {
		result = image_save_gif(imageout, ((format_params_gif)params)->interlace);
	}
	else if(type == FORMAT_ICON) {
		result = image_save_icon(imageout);
	}
	else if(type == FORMAT_JPEG) {
		result = image_save_jpeg(
			imageout, 
			((format_params_jpeg)params)->quality, 
			((format_params_jpeg)params)->smoothing, 
			((format_params_jpeg)params)->entropy, 
			((format_params_jpeg)params)->progressive,
			((format_params_jpeg)params)->comment,
			((format_params_jpeg)params)->subsampling,
			((format_params_jpeg)params)->baseline,
			((format_params_jpeg)params)->markers,
			((format_params_jpeg)params)->dct
		);
	}
	else if(type == FORMAT_PNG) {
		result = image_save_png(imageout, 
			((format_params_png)params)->interlace, 
			((format_params_png)params)->compression,
			((format_params_png)params)->savebgc,
			((format_params_png)params)->savegamma,
			((format_params_png)params)->saveoff,
			((format_params_png)params)->savephys,
			((format_params_png)params)->savetime,
			((format_params_png)params)->savecomm,
			((format_params_png)params)->savetrans
		);
	}
	else if(type == FORMAT_TGA) {
		result = image_save_tga(imageout, ((format_params_tga)params)->rle, ((format_params_tga)params)->origin);
	}
	else if(type == FORMAT_TIFF) {
		result = image_save_tiff(imageout, ((format_params_tiff)params)->compression);
	}
	else {
		result = gimp_file_save(
			GIMP_RUN_NONINTERACTIVE, 
			imageout->image_id, 
			imageout->drawable_id, 
			imageout->filepath, 
			imageout->filename
		);
	}
	
	return result;
}

static gboolean image_save_bmp(image_output out) 
{
	gint nreturn_vals;
	GimpParam *return_vals = gimp_run_procedure(
			"file_bmp_save",
			&nreturn_vals,
			GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
			GIMP_PDB_IMAGE, out->image_id,
			GIMP_PDB_DRAWABLE, out->drawable_id,
			GIMP_PDB_STRING, out->filepath,
			GIMP_PDB_STRING, out->filename,
			GIMP_PDB_END
		);
	
	return TRUE;
}

static gboolean image_save_gif(image_output out, gboolean interlace) 
{
	gint nreturn_vals;
	
	/* first, convert to indexed-256 color mode */
	gimp_image_convert_indexed(
		out->image_id,
		GIMP_FS_DITHER,
		GIMP_MAKE_PALETTE,
		gimp_drawable_has_alpha (out->drawable_id) ? 255 : 256,
		TRUE,
		FALSE,
		""
	);
	
	GimpParam *return_vals = gimp_run_procedure(
			"file_gif_save",
			&nreturn_vals,
			GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
			GIMP_PDB_IMAGE, out->image_id,
			GIMP_PDB_DRAWABLE, out->drawable_id,
			GIMP_PDB_STRING, out->filepath,
			GIMP_PDB_STRING, out->filename,
			GIMP_PDB_INT32, interlace ? 1 : 0,	/* Try to save as interlaced */
			GIMP_PDB_INT32, 1,					/* (animated gif) loop infinitely */
			GIMP_PDB_INT32, 0,					/* (animated gif) Default delay between framese in milliseconds */
			GIMP_PDB_INT32, 0,					/* (animated gif) Default disposal type (0=don't care, 1=combine, 2=replace) */
			GIMP_PDB_END
		);
		
	return TRUE;
}

static gboolean image_save_icon(image_output out) 
{
	gint nreturn_vals;
	GimpParam *return_vals = gimp_run_procedure(
			"file_ico_save",
			&nreturn_vals,
			GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
			GIMP_PDB_IMAGE, out->image_id,
			GIMP_PDB_DRAWABLE, out->drawable_id,
			GIMP_PDB_STRING, out->filepath,
			GIMP_PDB_STRING, out->filename,
			GIMP_PDB_END
		);
		
	return TRUE;
}

static gboolean image_save_jpeg(image_output out, float quality, float smoothing, gboolean entropy, gboolean progressive, gchar* comment, int subsampling, gboolean baseline, int markers, int dct) 
{
	gint nreturn_vals;
	
	/* image needs to be RGB */
	if (!gimp_drawable_is_rgb(out->drawable_id)) {
		gimp_image_convert_rgb(out->image_id);
	}
	
	GimpParam *return_vals = gimp_run_procedure(
			"file_jpeg_save",
			&nreturn_vals,
			GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
			GIMP_PDB_IMAGE, out->image_id,
			GIMP_PDB_DRAWABLE, out->drawable_id,
			GIMP_PDB_STRING, out->filepath,
			GIMP_PDB_STRING, out->filename,
			GIMP_PDB_FLOAT, quality >= 3 ? quality/100 : 0.03,	/* Quality of saved image (0 <= quality <= 1) + small fix because final image doesn't change when quality < 3 */
			GIMP_PDB_FLOAT, smoothing,				/* Smoothing factor for saved image (0 <= smoothing <= 1) */
			GIMP_PDB_INT32, entropy ? 1 : 0,		/* Optimization of entropy encoding parameters (0/1) */
			GIMP_PDB_INT32, progressive ? 1 : 0,	/* Enable progressive jpeg image loading - ignored if not compiled with HAVE_PROGRESSIVE_JPEG (0/1) */
			GIMP_PDB_STRING, comment,				/* Image comment */
			GIMP_PDB_INT32, subsampling,			/* The subsampling option number */
			GIMP_PDB_INT32, baseline ? 1 : 0,		/* Force creation of a baseline JPEG (non-baseline JPEGs can't be read by all decoders) (0/1) */
			GIMP_PDB_INT32, markers,				/* Frequency of restart markers (in rows, 0 = no restart markers) */
			GIMP_PDB_INT32, dct,					/* DCT algorithm to use (speed/quality tradeoff) */
			GIMP_PDB_END
		);
	
	return TRUE;
}

static gboolean image_save_png(image_output out, gboolean interlace, int compression, gboolean savebgc, gboolean savegamma, gboolean saveoff, gboolean savephys, gboolean savetime, gboolean savecomm, gboolean savetrans) 
{	
	gint nreturn_vals;
	GimpParam *return_vals = gimp_run_procedure(
			"file_png_save2",
			&nreturn_vals,
			GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
			GIMP_PDB_IMAGE, out->image_id,
			GIMP_PDB_DRAWABLE, out->drawable_id,
			GIMP_PDB_STRING, out->filepath,
			GIMP_PDB_STRING, out->filename,
			GIMP_PDB_INT32, interlace? 1 : 0,	/* Use Adam7 interlacing? */
			GIMP_PDB_INT32, compression,		/* Deflate Compression factor (0-9) */
			GIMP_PDB_INT32, savebgc? 1 : 0,		/* Write bKGD chunk? */
			GIMP_PDB_INT32, savegamma? 1 : 0,	/* Write gAMA chunk? */
			GIMP_PDB_INT32, saveoff? 1 : 0,		/* Write oFFs chunk? */
			GIMP_PDB_INT32, savephys? 1 : 0,	/* Write phys chunk? */
			GIMP_PDB_INT32, savetime? 1 : 0,	/* Write tIME chunk? */
			GIMP_PDB_INT32, savecomm? 1 : 0,	/* Write comments chunk? */
			GIMP_PDB_INT32, savetrans? 1 : 0,	/* Write trans chunk? */
			GIMP_PDB_END
		);
		
	return TRUE;
}

static gboolean image_save_tga(image_output out, gboolean rle, int origin) 
{
	gint nreturn_vals;
	GimpParam *return_vals = gimp_run_procedure(
			"file_tga_save",
			&nreturn_vals,
			GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
			GIMP_PDB_IMAGE, out->image_id,
			GIMP_PDB_DRAWABLE, out->drawable_id,
			GIMP_PDB_STRING, out->filepath,
			GIMP_PDB_STRING, out->filename,
			GIMP_PDB_INT32, rle? 1 : 0,	/* Use RLE compression */
			GIMP_PDB_INT32, origin,		/* Image origin */
			GIMP_PDB_END
		);
		
	return TRUE;
}

static gboolean image_save_tiff(image_output out, int compression) 
{
	gint nreturn_vals;
	GimpParam *return_vals = gimp_run_procedure(
			"file_tiff_save",
			&nreturn_vals,
			GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
			GIMP_PDB_IMAGE, out->image_id,
			GIMP_PDB_DRAWABLE, out->drawable_id,
			GIMP_PDB_STRING, out->filepath,
			GIMP_PDB_STRING, out->filename,
			GIMP_PDB_INT32, compression,	/* Compression type: { NONE (0), LZW (1), PACKBITS (2), DEFLATE (3), JPEG (4) } */
			GIMP_PDB_END
		);
		
	return TRUE;
}

/* returns a result code following this schema:
 * 0 = user responses "don't overwite" to a confirm dialog
 * 1 = old file was the same as the new one and user response "yes, overwite"
 * 2 = old file wasn't the same (implicit overwrite)
 */
static int overwrite_result(char* path, GtkWidget* parent) {
	gboolean oldfile_access = g_file_test(path, G_FILE_TEST_IS_REGULAR);//(access(path, F_OK) == 0);
	
	if (bimp_alertoverwrite && oldfile_access) {
		GtkWidget *dialog;
		dialog = gtk_message_dialog_new(
			GTK_WINDOW(parent),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_NONE,
			_("File %s already exists, overwrite it?"), bimp_comp_get_filename(path)
		);
		
		gtk_dialog_add_buttons (
			GTK_DIALOG(dialog),
			GTK_STOCK_YES, GTK_RESPONSE_YES,
			_("Yes, don't ask again"), GTK_RESPONSE_APPLY,
			GTK_STOCK_NO, GTK_RESPONSE_NO, NULL
		);
		
		gtk_window_set_title(GTK_WINDOW(dialog), _("Overwrite?"));
		gint result = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		
		if (result == GTK_RESPONSE_APPLY) {
			bimp_alertoverwrite = FALSE;
			return 1;
		} else if (result == GTK_RESPONSE_YES) {
			return 1;
		} else {
			return 0;
		}
	}
	else {
		return (oldfile_access ? 1 : 2);
	}
}

/* gets the current date and time in "%Y-%m-%d_%H-%M" format */
static char* get_datetime() {
	time_t rawtime;
	struct tm * timeinfo;
	char* format;

	format = (char*)malloc(sizeof(char)*18);
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	strftime (format, 18, "%Y-%m-%d_%H-%M", timeinfo);

	return format;
}
