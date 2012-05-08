/*
 * Functions called when the users clicks on 'APPLY'
 */

#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include "bimp-operate.h"
#include "bimp.h"
#include "bimp-manipulations.h"
#include "bimp-gui.h"

static void process_image(char*, gpointer);
static void apply_drawing_manipulation(manipulation, image_output);
static gboolean apply_resize(resize_settings, image_output);
static gboolean apply_crop(crop_settings, image_output);
static gboolean apply_fliprotate(fliprotate_settings, image_output);
static gboolean apply_color(color_settings, image_output);
static gboolean apply_sharpblur(sharpblur_settings, image_output);
static gboolean apply_watermark(watermark_settings, image_output);
static gboolean apply_userdef(userdef_settings, image_output);
static gboolean apply_rename(rename_settings, image_output, char*);
static gboolean image_save_bmp(image_output);
static gboolean image_save_gif(image_output, gboolean);
static gboolean image_save_icon(image_output);
static gboolean image_save_jpeg(image_output, float, float, gboolean, gboolean, gchar*, int, gboolean, int, int);
static gboolean image_save_png(image_output, gboolean, int);
static gboolean image_save_raw(image_output);
static gboolean image_save_tga(image_output, gboolean);
static gboolean image_save_tiff(image_output, int);
static gboolean ask_overwrite(char*, GtkWidget*);

/* utils */
static char* str_replace(char*, char*, char*);
static char* comp_get_filename(char*);
static char* get_datetime();
static float min(float, float);

int filecount, totalfilecount;
char* current_datetime;

gboolean bimp_start_batch(GtkWidget* parent) 
{
	bimp_set_busy(TRUE);
	
	g_print("\nBIMP - Batch Manipulation Plugin\nStart batch processing...\n");
	bimp_progress_bar_set(0.0, "");
	
	current_datetime = get_datetime(); /* used by apply_rename */
	filecount = 0;
	totalfilecount = g_slist_length(bimp_input_filenames);
	g_slist_foreach(bimp_input_filenames, (GFunc)process_image, parent);
	
	bimp_progress_bar_set(100.0, "End, all files processed");
	g_print("\nEnd, %d files processed.\n", filecount);
	
	bimp_set_busy(FALSE);
	
	return TRUE;
}

static void process_image(char* file, gpointer parent) 
{
	image_output imageout;
	char* orig_basename;
	char* orig_file_ext;
	
	imageout = (image_output)g_malloc(sizeof(struct imageout_str));
	
	orig_basename = g_strdup(comp_get_filename(file)); /* store original filename */
	orig_file_ext = g_strdup(strrchr(orig_basename, '.')); /* store original extension */
	
	/* load the file and assign ids */
	bimp_progress_bar_set((float)filecount/totalfilecount, g_strconcat("Working on file ", orig_basename, "...", NULL));
	g_print("\nLoading file #%d: %s (%s)\n", filecount+1, orig_basename, file);
	
	imageout->image_id = gimp_file_load(GIMP_RUN_NONINTERACTIVE, (gchar*)file, (gchar*)orig_basename); /* load file and get image id */
	g_print("Image ID is %d\n", imageout->image_id);
	
	imageout->drawable_id = gimp_image_merge_visible_layers(imageout->image_id, GIMP_CLIP_TO_IMAGE); /* merge levels and get drawable id */
	gimp_layer_add_alpha (imageout->drawable_id);
	g_print("Drawable ID is %d\n", imageout->drawable_id);
	
	/* start manipulations sequence, cropping and scaling start first */
	if (bimp_list_contains_manip(MANIP_CROP)) {
		g_print("Applying CROP...\n");
		apply_crop((crop_settings)(bimp_list_get_manip(MANIP_CROP))->settings, imageout);
	}
	
	if (bimp_list_contains_manip(MANIP_RESIZE)) {
		g_print("Applying RESIZE...\n");
		apply_resize((resize_settings)(bimp_list_get_manip(MANIP_RESIZE))->settings, imageout);
	}
	
	/* apply all the intermediate manipulations */
	g_slist_foreach(bimp_selected_manipulations, (GFunc)apply_drawing_manipulation, imageout);
	
	/*  watermark at last */
	if(bimp_list_contains_manip(MANIP_WATERMARK)) {
		g_print("Applying WATERMARK...\n");
		apply_watermark((watermark_settings)(bimp_list_get_manip(MANIP_WATERMARK))->settings, imageout);
	}
	
	orig_basename[strlen(orig_basename) - strlen(orig_file_ext)] = '\0'; /* remove extension from basename */
	
	/* check if a rename pattern is defined */
	if(bimp_list_contains_manip(MANIP_RENAME)) {
		g_print("Applying RENAME...\n");
		apply_rename((rename_settings)(bimp_list_get_manip(MANIP_RENAME))->settings, imageout, orig_basename);
	}
	else {
		imageout->filename = orig_basename;
	}
	
	/* save the final image in output dir with proper format */
	if(bimp_list_contains_manip(MANIP_CHANGEFORMAT)) {
		changeformat_settings settings = (changeformat_settings)(bimp_list_get_manip(MANIP_CHANGEFORMAT))->settings;
		format_type type = settings->format;
		format_params params = settings->params;
		
		g_print("Changing FORMAT to %s\n", format_type_string[type][0]);
		imageout->filename = g_strconcat(imageout->filename, ".", format_type_string[type][0], NULL); /* append new file extension */
		imageout->filepath = g_strconcat(bimp_output_folder, "/", imageout->filename, NULL); /* build new path */
		g_print("Saving file %s in %s\n", imageout->filename, imageout->filepath);
		
		
		if (ask_overwrite(imageout->filepath, parent)) {
			if (type == FORMAT_BMP) {
				image_save_bmp(imageout);
			}
			else if(type == FORMAT_GIF) {
				image_save_gif(imageout, ((format_params_gif)params)->interlace);
			}
			else if(type == FORMAT_ICON) {
				image_save_icon(imageout);
			}
			else if(type == FORMAT_JPEG) {
				image_save_jpeg(
					imageout, 
					((format_params_jpeg)params)->quality, 
					((format_params_jpeg)params)->smoothing, 
					((format_params_jpeg)params)->entropy, 
					((format_params_jpeg)params)->progressive,
					((format_params_jpeg)params)->comment,
					((format_params_jpeg)params)->subsampling,
					((format_params_jpeg)params)->baseline,
					((format_params_jpeg)params)->markers,
					((format_params_jpeg)params)->dct);
			}
			else if(type == FORMAT_PNG) {
				image_save_png(imageout, ((format_params_png)params)->interlace, ((format_params_png)params)->compression);
			}
			/*else if(type == FORMAT_RAW) {
				image_save_raw(imageout);
			}*/
			else if(type == FORMAT_TGA) {
				image_save_tga(imageout, ((format_params_tga)params)->rle);
			}
			else if(type == FORMAT_TIFF) {
				image_save_tiff(imageout, ((format_params_tiff)params)->compression);
			}
		}
		else {
			g_print("File has not been overwritten!\n");
		}
	}
	else {
		/* if not specified, save in original format */
		imageout->filename = g_strconcat(imageout->filename, orig_file_ext, NULL); /* append old file extension */
		imageout->filepath = g_strconcat(bimp_output_folder, "/", imageout->filename, NULL); /* build new path */
		
		if (ask_overwrite(imageout->filepath, parent)) {
			g_print("Saving file %s in %s\n", imageout->filename, imageout->filepath);
			gimp_file_save(
				GIMP_RUN_NONINTERACTIVE, 
				imageout->image_id, 
				imageout->drawable_id, 
				imageout->filepath, 
				imageout->filename
			);
		}
		else {
			g_print("File has not been overwritten!\n");
		}
	}
	
	gimp_image_delete(imageout->image_id); /* is it useful? */
	g_free(orig_basename);
	g_free(orig_file_ext);
	g_free(imageout);
		
	filecount ++;
	bimp_progress_bar_set((double)filecount/totalfilecount, NULL);
}

static void apply_drawing_manipulation(manipulation man, image_output out) 
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
}

static gboolean apply_resize(resize_settings settings, image_output out) 
{
	gboolean success = FALSE;
	gint finalW, finalH;
	gdouble origResX, origResY;
	
	if (settings->change_res) {
		success = gimp_image_get_resolution(
			out->image_id,
			&origResX,
			&origResY
		);
		
		if ((settings->newResX != origResX) || (settings->newResY != origResY)) {
			/* change resolution */
			success = gimp_image_set_resolution(
				out->image_id,
				settings->newResX,
				settings->newResY
			);
		}
	}
	
	
	if (settings->sizemode == RESIZE_PERCENT) {
		/* user selected a percentage of the original size */
		finalW = round((gimp_image_width(out->image_id) * settings->newWpc) / 100.0);
		finalH = round((gimp_image_height(out->image_id) * settings->newHpc) / 100.0);
	}
	else {
		/* user typed exact pixel size */
		if (settings->sizemode == RESIZE_PIXEL_WIDTH) {
			/* width only */
			finalW = settings->newWpx;
			if (settings->aspect_ratio) {
				/* and maintain aspect ratio */
				finalH = round(((float)settings->newWpx * gimp_image_height(out->image_id)) / gimp_image_width(out->image_id));
			}
			else {
				finalH = gimp_image_height(out->image_id);
			}
		}
		else if (settings->sizemode == RESIZE_PIXEL_HEIGHT) {
			/* height only */
			finalH = settings->newHpx;
			if (settings->aspect_ratio) {
				/* and maintain aspect ratio */
				finalW = round(((float)settings->newHpx * gimp_image_width(out->image_id)) / gimp_image_height(out->image_id));
			}
			else {
				finalW = gimp_image_width(out->image_id);
			}
		}
		else {
			/* both dimensions are defined */
			finalW = settings->newWpx;
			finalH = settings->newHpx;
		}
	}
	
	/* do resize */
	success = gimp_image_scale_full (
		out->image_id, 
		finalW, 
		finalH, 
		settings->interpolation
	);
	
	return success;
}

static gboolean apply_crop(crop_settings settings, image_output out) 
{
	gboolean success = TRUE;
	gint newWidth, newHeight, oldWidth, oldHeight, posX, posY;
	
	oldWidth = gimp_image_width(out->image_id);
	oldHeight = gimp_image_height(out->image_id);
	
	if (settings->manual) {
		newWidth = settings->newW;
		newHeight = settings->newH;
		posX = 0;
		posY = 0;
	}
	else {
		if (( (float)oldWidth / oldHeight ) > ( (float)crop_preset_ratio[settings->ratio][0] / crop_preset_ratio[settings->ratio][1]) ) { 
			/* crop along the width */
			newHeight = oldHeight;
			newWidth = round(( crop_preset_ratio[settings->ratio][0] * (float)newHeight ) / crop_preset_ratio[settings->ratio][1]);
			posX = (oldWidth - newWidth) / 2;
			posY = 0;
		} else { 
			/* crop along the height */
			newWidth = oldWidth;
			newHeight = round(( crop_preset_ratio[settings->ratio][1] * (float)newWidth) / crop_preset_ratio[settings->ratio][0]);
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
	
	if (settings->flipH) {
		/* do horizontal flip */
		success = gimp_image_flip (
			out->image_id,
			GIMP_ORIENTATION_HORIZONTAL
		);
	}
	
	if (settings->flipV) {
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
			settings->rotate_type
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
		float minsize = min(gimp_image_width(out->image_id)/4, gimp_image_height(out->image_id)/4) ;
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
	
	if (settings->textmode) {
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
		if ((access(settings->imagefile, R_OK) == -1)) {
			return TRUE;
		}
		
		layerId = gimp_file_load_layer(
			GIMP_RUN_NONINTERACTIVE,
			out->image_id,
			settings->imagefile
		);
		
		gimp_layer_set_opacity(layerId, settings->opacity);
		wmwidth = gimp_drawable_width(layerId);
		wmheight = gimp_drawable_height(layerId);
		
		gimp_image_add_layer(
			out->image_id,
			layerId,
            0
        );
        
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
	char *orig_filename = g_strdup(orig_basename);
	
	out->filename = g_strdup(settings->pattern);
	
	/* search for 'RENAME_KEY_ORIG' occurrences and replace the final filename */
	if(strstr(out->filename, RENAME_KEY_ORIG) != NULL) {
		out->filename = str_replace(out->filename, RENAME_KEY_ORIG, orig_filename);
	}
	
	/* same thing for count and datetime */
	
	if(strstr(out->filename, RENAME_KEY_COUNT) != NULL)	{
		char strcount[5];
		sprintf(strcount, "%i", filecount + 1);
		out->filename = str_replace(out->filename, RENAME_KEY_COUNT, strcount);
	}
	
	if(strstr(out->filename, RENAME_KEY_DATETIME) != NULL)	{
		out->filename = str_replace(out->filename, RENAME_KEY_DATETIME, current_datetime);
	}
	
	g_free(orig_filename);
	g_free(orig_filename);
	return TRUE;
}

/* following: set of functions that saves the image file in various formats */

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

static gboolean image_save_png(image_output out, gboolean interlace, int compression) 
{	
	gint nreturn_vals;
	GimpParam *return_vals = gimp_run_procedure(
			"file_png_save",
			&nreturn_vals,
			GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
			GIMP_PDB_IMAGE, out->image_id,
			GIMP_PDB_DRAWABLE, out->drawable_id,
			GIMP_PDB_STRING, out->filepath,
			GIMP_PDB_STRING, out->filename,
			GIMP_PDB_INT32, interlace? 1 : 0,	/* Use Adam7 interlacing? */
			GIMP_PDB_INT32, compression,		/* Deflate Compression factor (0--9) */
			GIMP_PDB_INT32, 1,					/* Write bKGD chunk? */
			GIMP_PDB_INT32, 1,					/* Write gAMA chunk? */
			GIMP_PDB_INT32, 1,					/* Write oFFs chunk? */
			GIMP_PDB_INT32, 1,					/* Write tIME chunk? */
			GIMP_PDB_INT32, 1,					/* Write pHYs chunk? */
			GIMP_PDB_END
		);
		
	return TRUE;
}

/* NEED FIX! */
static gboolean image_save_raw(image_output out) 
{
	gint nreturn_vals;
	GimpParam *return_vals = gimp_run_procedure(
			"file_raw_save",
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

static gboolean image_save_tga(image_output out, gboolean rle) 
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

/* if global var 'bimp_alertoverwrite' is TRUE and the file at 'path' exists, 
 * asks for overwrite it with a dialog */
static gboolean ask_overwrite(char* path, GtkWidget* parent) {
	gboolean can_overwrite = TRUE;
	
	if (bimp_alertoverwrite && access(path, F_OK) == 0) {
		GtkWidget *dialog;
		dialog = gtk_message_dialog_new(
			GTK_WINDOW(parent),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			"File %s already exists,\noverwrite it?", comp_get_filename(path));
		  gtk_window_set_title(GTK_WINDOW(dialog), "Overwrite?");
		  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
		  gtk_widget_destroy(dialog);
		  
		  can_overwrite = (result == GTK_RESPONSE_YES);
	}
	
	return can_overwrite;
}

/* replace all the occurrences of 'rep' into 'orig' with text 'with' */
static char *str_replace(char *orig, char *rep, char *with) 
{
    char *result;
    char *ins;
    char *tmp;
    int len_rep;
    int len_with;
    int len_front;
    int count;

    if (!orig) {
        return NULL;
	}
    if (!rep || !(len_rep = strlen(rep))) {
        return NULL;
	}
    if (!(ins = strstr(orig, rep))) {
        return NULL;
	}
	
    if (!with) {
        with = "";
    }
    
    len_with = strlen(with);

    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result) {
        return NULL;
	}
	
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep;
    }
    strcpy(tmp, orig);
    return result;
}

/* gets the filename from the given path (compatible with unix and win) */
static char* comp_get_filename(char* path) 
{
	char *pfile;
	
	pfile = path + strlen(path);
    for (; pfile > path; pfile--)
    {
        if ((*pfile == '\\') || (*pfile == '/'))
        {
            pfile++;
            break;
        }
    }
    
    return pfile;
}

/* gets the current date and time in format "%Y-%m-%d_%H-%M".
 * used by RENAME manipulation */
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

static float min(float a, float b) {
	if (a < b)
		return a;
	else
		return b;
}
