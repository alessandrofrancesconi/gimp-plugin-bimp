/*
 * Functions to serialize and load manipulation sets
 * Serialization uses GKeyFile method
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include "bimp.h"
#include "bimp-manipulations.h"
#include "bimp-utils.h"
#include "bimp-serialize.h"

static void append_manipulation_details(manipulation, GKeyFile*);
static GSList* parse_manipulations(GKeyFile*, int); 
static void write_resize(resize_settings, GKeyFile*);
static manipulation read_resize(GKeyFile*);
static void write_crop(crop_settings, GKeyFile*);
static manipulation read_crop(GKeyFile*);
static void write_fliprotate(fliprotate_settings, GKeyFile*);
static manipulation read_fliprotate(GKeyFile*);
static void write_color(color_settings, GKeyFile*);
static manipulation read_color(GKeyFile*, int);
static void write_sharpblur(sharpblur_settings, GKeyFile*);
static manipulation read_sharpblur(GKeyFile*);
static void write_watermark(watermark_settings, GKeyFile*);
static manipulation read_watermark(GKeyFile*);
static void write_changeformat(changeformat_settings, GKeyFile*);
static manipulation read_changeformat(GKeyFile*);
static void write_rename(rename_settings, GKeyFile*);
static manipulation read_rename(GKeyFile*);
static void write_userdef(userdef_settings, GKeyFile*, int);
static manipulation read_userdef(GKeyFile*, int);

static int userdef_count;
static int loaded_build;

gboolean bimp_serialize_to_file(gchar* filename)
{
    gboolean result;
    
    GKeyFile *output_file = g_key_file_new();
    g_key_file_set_list_separator(output_file, ';');
    
    g_key_file_set_comment (output_file, NULL, NULL, g_strdup_printf("BIMP %s\nMANIPULATION SET DEFINITION", PLUG_IN_VERSION), NULL);
    
    userdef_count = 0;
    g_slist_foreach(bimp_selected_manipulations, (GFunc)append_manipulation_details, output_file);
    
    result = g_file_set_contents (filename, g_key_file_to_data(output_file, NULL, NULL), -1, NULL);
    
    g_key_file_free(output_file);
    
    return result;
}

gboolean bimp_deserialize_from_file(gchar* filename)
{
    gboolean result;
    
    GKeyFile* input_file = g_key_file_new();
    g_key_file_set_list_separator(input_file, ';');
    
    if ((result = g_key_file_load_from_file (input_file, filename, G_KEY_FILE_KEEP_COMMENTS, NULL))) {
        
		// read build code
		int buildnumber = 0;
		gchar* header = g_key_file_get_comment(input_file, NULL, NULL, NULL);
		GRegex *regex = g_regex_new ("^BIMP\\s(\\d+)\\.(\\d+)", 0, 0, NULL);
		
		GMatchInfo *match_info;
		g_regex_match (regex, header, 0, &match_info);
		while (g_match_info_matches (match_info))
		{
			gchar *major = g_match_info_fetch (match_info, 1);
			int major_i = g_ascii_strtoll(major, NULL, 10);
			gchar *minor = g_match_info_fetch (match_info, 2);
			int minor_i = g_ascii_strtoll(major, NULL, 10);
			buildnumber = (major_i * 1000) + minor_i;
			if (buildnumber > 0) break;
			
			g_match_info_next (match_info, NULL);
		}
		g_match_info_free (match_info);
		g_regex_unref (regex);
		
        GSList* new_list = parse_manipulations(input_file, buildnumber);
        if (new_list != NULL) {
            g_slist_free(bimp_selected_manipulations);
            bimp_selected_manipulations = new_list;
            
            result = TRUE;
        }
        else {
            result = FALSE;
        }
    }
    
    g_key_file_free(input_file);
    
    return result;
}

static void append_manipulation_details(manipulation man, GKeyFile* output_file) 
{
    if (man->type == MANIP_RESIZE) {
        write_resize((resize_settings)man->settings, output_file);
    }
    else if (man->type == MANIP_CROP) {
        write_crop((crop_settings)man->settings, output_file);
    }
    else if (man->type == MANIP_FLIPROTATE) {
        write_fliprotate((fliprotate_settings)man->settings, output_file);
    }
    else if (man->type == MANIP_COLOR) {
        write_color((color_settings)man->settings, output_file);
    }
    else if (man->type == MANIP_SHARPBLUR) {
        write_sharpblur((sharpblur_settings)man->settings, output_file);
    }
    else if (man->type == MANIP_WATERMARK) {
        write_watermark((watermark_settings)man->settings, output_file);
    }
    else if (man->type == MANIP_CHANGEFORMAT) {
        write_changeformat((changeformat_settings)man->settings, output_file);
    }
    else if (man->type == MANIP_RENAME) {
        write_rename((rename_settings)man->settings, output_file);
    }
    else if (man->type == MANIP_USERDEF) {
        write_userdef((userdef_settings)man->settings, output_file, userdef_count);
        userdef_count++;
    }
}

static GSList* parse_manipulations(GKeyFile* file, int version) 
{
    GSList* manipulations = NULL;
    manipulation newman = NULL;
    gsize count = 0;
    gchar** groups;
    
    groups = g_key_file_get_groups(file, &count);
    int i = 0;
    while (i < count) {
        
        if (strcmp(groups[i], "RESIZE") == 0) {
            newman = read_resize(file);
        }
        else if (strcmp(groups[i], "CROP") == 0) {
            newman = read_crop(file);
        }
        else if (strcmp(groups[i], "FLIPROTATE") == 0) {
            newman = read_fliprotate(file);
        }
        else if (strcmp(groups[i], "COLOR") == 0) {
            newman = read_color(file, version);
        }
        else if (strcmp(groups[i], "SHARPBLUR") == 0) {
            newman = read_sharpblur(file);
        }
        else if (strcmp(groups[i], "WATERMARK") == 0) {
            newman = read_watermark(file);
        }
        else if (strcmp(groups[i], "CHANGEFORMAT") == 0) {
            newman = read_changeformat(file);
        }
        else if (strcmp(groups[i], "RENAME") == 0) {
            newman = read_rename(file);
        }
        else if (strncmp(groups[i], "USERDEF", strlen("USERDEF")) == 0) {
            int userdef_id;
            if (sscanf(groups[i], "USERDEF%d", &userdef_id) == 1) {
                newman = read_userdef(file, userdef_id);
            }
        }
        
        if (newman != NULL) manipulations = g_slist_append(manipulations, newman);
        
        i++;
    }
    
    return manipulations;
}

static void write_resize(resize_settings settings, GKeyFile* file) 
{
    gchar* group_name = "RESIZE";
    
    g_key_file_set_double(file, group_name, "new_w_pc", settings->new_w_pc);
    g_key_file_set_double(file, group_name, "new_h_pc", settings->new_h_pc);
    g_key_file_set_integer(file, group_name, "new_w_px", settings->new_w_px);
    g_key_file_set_integer(file, group_name, "new_h_px", settings->new_h_px);
    g_key_file_set_integer(file, group_name, "resize_mode_width", settings->resize_mode_width);
    g_key_file_set_integer(file, group_name, "resize_mode_height", settings->resize_mode_height);
    g_key_file_set_integer(file, group_name, "stretch_mode", settings->stretch_mode);
    g_key_file_set_string(file, group_name, "padding_color", gdk_color_to_string(&(settings->padding_color)));
    g_key_file_set_integer(file, group_name, "padding_color_alpha", settings->padding_color_alpha);
    g_key_file_set_integer(file, group_name, "interpolation", settings->interpolation);
    g_key_file_set_boolean(file, group_name, "change_res", settings->change_res);
    g_key_file_set_integer(file, group_name, "new_res_x", settings->new_res_x);
    g_key_file_set_integer(file, group_name, "new_res_y", settings->new_res_y);
}

/* deserializes a string and returns a resize manipulation */
static manipulation read_resize(GKeyFile* file) 
{
    gchar* group_name = "RESIZE";
    manipulation man = NULL;
    
    if (g_key_file_has_group(file, group_name)) {
        man = manipulation_resize_new();
        resize_settings settings = ((resize_settings)man->settings);
        
        if (g_key_file_has_key(file, group_name, "new_w_pc", NULL)) 
            settings->new_w_pc = g_key_file_get_double(file, group_name, "new_w_pc", NULL);
            
        if (g_key_file_has_key(file, group_name, "new_h_pc", NULL)) 
            settings->new_h_pc = g_key_file_get_double(file, group_name, "new_h_pc", NULL);
            
        if (g_key_file_has_key(file, group_name, "new_w_px", NULL)) 
            settings->new_w_px = g_key_file_get_integer(file, group_name, "new_w_px", NULL);
            
        if (g_key_file_has_key(file, group_name, "new_h_px", NULL)) 
            settings->new_h_px = g_key_file_get_integer(file, group_name, "new_h_px", NULL);
            
        // for backwards compatibility
        if (g_key_file_has_key(file, group_name, "resize_mode", NULL)) {
            switch(g_key_file_get_integer(file, group_name, "resize_mode", NULL)) {
                case 0: // RESIZE_PERCENT
                    settings->resize_mode_width = settings->resize_mode_height = RESIZE_PERCENT;
                    break;
                case 1: // RESIZE_PIZEL_BOTH
                    settings->resize_mode_width = settings->resize_mode_height = RESIZE_PIXEL;
                    break;
                case 2: // RESIZE_PIXEL_WIDTH
                    settings->resize_mode_width = RESIZE_PIXEL;
                    settings->resize_mode_height = RESIZE_DISABLE;
                    break;
                case 3: // RESIZE_PIXEL_HEIGHT
                    settings->resize_mode_width = RESIZE_DISABLE;
                    settings->resize_mode_height = RESIZE_PIXEL;
                    break;
                case 4: // RESIZE_END
                default:
                    settings->resize_mode_width = settings->resize_mode_height = RESIZE_END;
            }
        }
            
        if (g_key_file_has_key(file, group_name, "resize_mode_width", NULL)) 
            settings->resize_mode_width = g_key_file_get_integer(file, group_name, "resize_mode_width", NULL);
            
        if (g_key_file_has_key(file, group_name, "resize_mode_height", NULL)) 
            settings->resize_mode_height = g_key_file_get_integer(file, group_name, "resize_mode_height", NULL);
            
        if (g_key_file_has_key(file, group_name, "stretch_mode", NULL)) 
            settings->stretch_mode = g_key_file_get_integer(file, group_name, "stretch_mode", NULL);
        
        if (g_key_file_has_key(file, group_name, "padding_color", NULL)) 
            gdk_color_parse(g_key_file_get_string(file, group_name, "padding_color", NULL), &(settings->padding_color));
        
        if (g_key_file_has_key(file, group_name, "padding_color_alpha", NULL)) 
            settings->padding_color_alpha = (guint16)g_key_file_get_integer(file, group_name, "padding_color_alpha", NULL);
        
        if (g_key_file_has_key(file, group_name, "interpolation", NULL)) 
            settings->interpolation = g_key_file_get_integer(file, group_name, "interpolation", NULL);
            
        if (g_key_file_has_key(file, group_name, "change_res", NULL)) 
            settings->change_res = g_key_file_get_boolean(file, group_name, "change_res", NULL);
            
        if (g_key_file_has_key(file, group_name, "new_res_x", NULL)) 
            settings->new_res_x = g_key_file_get_integer(file, group_name, "new_res_x", NULL);
            
        if (g_key_file_has_key(file, group_name, "new_res_y", NULL)) 
            settings->new_res_y = g_key_file_get_integer(file, group_name, "new_res_y", NULL);
    }
    
    return man;
}

static void write_crop(crop_settings settings, GKeyFile* file) 
{
    gchar* group_name = "CROP";
    
    g_key_file_set_integer(file, group_name, "new_w", settings->new_w);
    g_key_file_set_integer(file, group_name, "new_h", settings->new_h);
    g_key_file_set_boolean(file, group_name, "manual", settings->manual);
    g_key_file_set_integer(file, group_name, "ratio", settings->ratio);
    g_key_file_set_double(file, group_name, "custom_ratio1", settings->custom_ratio1);
    g_key_file_set_double(file, group_name, "custom_ratio2", settings->custom_ratio2);
    g_key_file_set_double(file, group_name, "start_pos", settings->start_pos);
}

static manipulation read_crop(GKeyFile* file) 
{
    gchar* group_name = "CROP";
    manipulation man = NULL;
    
    if (g_key_file_has_group(file, group_name)) {
        man = manipulation_crop_new();
        crop_settings settings = ((crop_settings)man->settings);
        
        if (g_key_file_has_key(file, group_name, "new_w", NULL)) 
            settings->new_w = g_key_file_get_integer(file, group_name, "new_w", NULL);
            
        if (g_key_file_has_key(file, group_name, "new_h", NULL)) 
            settings->new_h = g_key_file_get_integer(file, group_name, "new_h", NULL);
            
        if (g_key_file_has_key(file, group_name, "manual", NULL)) 
            settings->manual = g_key_file_get_boolean(file, group_name, "manual", NULL);
            
        if (g_key_file_has_key(file, group_name, "ratio", NULL)) 
            settings->ratio = g_key_file_get_integer(file, group_name, "ratio", NULL);
            
        if (g_key_file_has_key(file, group_name, "custom_ratio1", NULL)) 
            settings->custom_ratio1 = g_key_file_get_integer(file, group_name, "custom_ratio1", NULL);
            
        if (g_key_file_has_key(file, group_name, "custom_ratio2", NULL)) 
            settings->custom_ratio2 = g_key_file_get_integer(file, group_name, "custom_ratio2", NULL);
            
        if (g_key_file_has_key(file, group_name, "start_pos", NULL)) 
            settings->start_pos = g_key_file_get_integer(file, group_name, "start_pos", NULL);
    }
    
    return man;
}

static void write_fliprotate(fliprotate_settings settings, GKeyFile* file) 
{
    gchar* group_name = "FLIPROTATE";
    
    g_key_file_set_boolean(file, group_name, "flip_h", settings->flip_h);
    g_key_file_set_boolean(file, group_name, "flip_v", settings->flip_v);
    g_key_file_set_boolean(file, group_name, "rotate", settings->rotate);
    g_key_file_set_integer(file, group_name, "rotation_type", settings->rotation_type);
}

static manipulation read_fliprotate(GKeyFile* file) 
{
    gchar* group_name = "FLIPROTATE";
    manipulation man = NULL;
    
    if (g_key_file_has_group(file, group_name)) {
        man = manipulation_fliprotate_new();
        fliprotate_settings settings = ((fliprotate_settings)man->settings);
        
        if (g_key_file_has_key(file, group_name, "flip_h", NULL)) 
            settings->flip_h = g_key_file_get_boolean(file, group_name, "flip_h", NULL);
            
        if (g_key_file_has_key(file, group_name, "flip_v", NULL)) 
            settings->flip_v = g_key_file_get_boolean(file, group_name, "flip_v", NULL);
            
        if (g_key_file_has_key(file, group_name, "rotate", NULL)) 
            settings->rotate = g_key_file_get_boolean(file, group_name, "rotate", NULL);
            
        if (g_key_file_has_key(file, group_name, "rotation_type", NULL)) 
            settings->rotation_type = g_key_file_get_integer(file, group_name, "rotation_type", NULL);
    }
    
    return man;
}

static void write_color(color_settings settings, GKeyFile* file) 
{
    gchar* group_name = "COLOR";
    
    g_key_file_set_double(file, group_name, "brightness", settings->brightness);
    g_key_file_set_double(file, group_name, "contrast", settings->contrast);
    g_key_file_set_boolean(file, group_name, "levels_auto", settings->levels_auto);
    g_key_file_set_boolean(file, group_name, "grayscale", settings->grayscale);
    if (settings->curve_file != NULL) g_key_file_set_string(file, group_name, "curve_file", settings->curve_file);
}

static manipulation read_color(GKeyFile* file, int version) 
{
    gchar* group_name = "COLOR";
    manipulation man = NULL;
    
    if (g_key_file_has_group(file, group_name)) {
        man = manipulation_color_new();
        color_settings settings = ((color_settings)man->settings);
        
        if (g_key_file_has_key(file, group_name, "brightness", NULL)) 
		{
			double b = g_key_file_get_double(file, group_name, "brightness", NULL);
			if (version < 2000)
			{
				// prior to BIMP 2.0, brightness range was [-127;127]
				// now map to [-0.5;0.5]
				settings->brightness = -0.5 + ((0.5 - (-0.5)) / (127 - (-127)) * (b - (-127)));
			}
			else
			{
				settings->brightness = b;
			}
		}
            
        if (g_key_file_has_key(file, group_name, "contrast", NULL)) 
		{
			double c = g_key_file_get_double(file, group_name, "contrast", NULL);
			if (version < 2000)
			{
				// prior to BIMP 2.0, contrast range was [-127;127]
				// now map to [-0.5;0.5]
				settings->contrast = -0.5 + ((0.5 - (-0.5)) / (127 - (-127)) * (c - (-127)));
			}
			else
			{
				settings->contrast = c;
			}
		}
            
        if (g_key_file_has_key(file, group_name, "levels_auto", NULL)) 
            settings->levels_auto = g_key_file_get_boolean(file, group_name, "levels_auto", NULL);
            
        if (g_key_file_has_key(file, group_name, "grayscale", NULL)) 
            settings->grayscale = g_key_file_get_boolean(file, group_name, "grayscale", NULL);
            
        if (g_key_file_has_key(file, group_name, "curve_file", NULL)) 
            settings->curve_file = g_key_file_get_string(file, group_name, "curve_file", NULL);
    }
    
    return man;
}

/* Reads the content of a GIMP's curve file (for Color manipulations). 
 * The result is saved to be used with the gimp_curves_spline() function */
gboolean parse_curve_file(
    char* file, 
    int* num_points_v, 
    gdouble** ctr_points_v,
    int* num_points_r, 
    gdouble** ctr_points_r,
    int* num_points_g, 
    gdouble** ctr_points_g,
    int* num_points_b, 
    gdouble** ctr_points_b,
    int* num_points_a, 
    gdouble** ctr_points_a
) {
    FILE* pFile;
    pFile = fopen (file, "r");
    
    char* old_locale = setlocale(LC_NUMERIC, "");
    setlocale(LC_NUMERIC, "C");
    
    char line[2400];
    char channel_name[6];
    int num_points_temp = 0;
    gdouble* ctr_points_temp = NULL;
    
    if (pFile == NULL) goto err;
    else {
        // read header ("# GIMP curves tool settings")
        if (fgets(line, sizeof(line), pFile) == NULL) goto err;
        if (!g_str_has_prefix(line, "# GIMP")) goto err;
        
        if (fgets(line, sizeof(line), pFile) == NULL) goto err;
        while(!g_str_has_prefix(line, "(channel ")) {
            if (fgets(line, sizeof(line), pFile) == NULL) goto err;
        }
        
        // reached the first "(channel " line
        while (sscanf (line, "(channel %[a-z])", channel_name) == 1) {
            
            g_free(ctr_points_temp);
            ctr_points_temp = NULL;
            
            // "(curve", ignored
            if (fgets(line, sizeof(line), pFile) == NULL) goto err;
            if (g_str_has_prefix(line, "(curve")) {
                if (fgets(line, sizeof(line), pFile) == NULL) goto err;
            }
            // "    (curve-type", ignored
            if (g_str_has_prefix(line, "    (curve-type")) {
                if (fgets(line, sizeof(line), pFile) == NULL) goto err;
            }
            
            // n-points, ignored (also it is missing since GIMP 2.10)
            // the actual number of points is parsed from "(points N..."
            if (g_str_has_prefix(line, "    (n-points")) {
                if (fgets(line, sizeof(line), pFile) == NULL) goto err;
            }
            
            // number of points and list
            double pX, pY;
            int p_count = 0;
            char* token = strtok(line + strlen(g_strdup_printf("    (points ")), " ");
            int num_points = atoi(token); // first token holds the number of points
            
            token = strtok(NULL, " "); 
            while (token) {
                pX = atof(token);
                token = strtok(NULL, " ");
                if (!token) goto err;
                pY = atof(token);
                
                if (pX >= 0 && pX <= 1 &&
                    pY >= 0 && pY <= 1) 
                {
                    // save X and Y
                    ctr_points_temp = (gdouble*)g_realloc(ctr_points_temp, sizeof(gdouble) * (p_count + 2)); // add one element to the array
                    ctr_points_temp[p_count]     = pX;
                    ctr_points_temp[p_count + 1] = pY;
                    
                    p_count += 2;
                }
                
                token = strtok(NULL, " ");
            }
            
            num_points_temp = p_count;
            
            //    "(n-samples XX)", ignored
            if (g_str_has_prefix(line, "    (n-samples")) {
                if (fgets(line, sizeof(line), pFile) == NULL) goto err;
            }
            //    "(samples XX ...", ignored
            if (g_str_has_prefix(line, "    (samples")) {
                if (fgets(line, sizeof(line), pFile) == NULL) goto err;
            }
            
            // save in the proper variables
            if (strcmp(channel_name, "value") == 0) {
                *num_points_v = num_points_temp;
                *ctr_points_v = g_memdup(ctr_points_temp, num_points_temp * sizeof(gdouble));
            }
            else if (strcmp(channel_name, "red") == 0) {
                *num_points_r = num_points_temp;
                *ctr_points_r = g_memdup(ctr_points_temp, num_points_temp * sizeof(gdouble));
            }
            else if (strcmp(channel_name, "green") == 0) {
                *num_points_g = num_points_temp;
                *ctr_points_g = g_memdup(ctr_points_temp, num_points_temp * sizeof(gdouble));
            }
            else if (strcmp(channel_name, "blue") == 0) {
                *num_points_b = num_points_temp;
                *ctr_points_b = g_memdup(ctr_points_temp, num_points_temp * sizeof(gdouble));
            }
            else if (strcmp(channel_name, "alpha") == 0) {
                *num_points_a = num_points_temp;
                *ctr_points_a = g_memdup(ctr_points_temp, num_points_temp * sizeof(gdouble));
            }
            else goto err;
            
            // reach the next channel, or the end if this was the last
            if (fgets(line, sizeof(line), pFile) == NULL) goto err;
            while(!g_str_has_prefix(line, "(channel ")) {
                if (fgets(line, sizeof(line), pFile) == NULL) goto err;
                if (g_str_has_prefix(line, "# end")) goto finish; 
            }
        }
        
        // "# end of curves tool settings"
finish:
        setlocale(LC_NUMERIC, old_locale);
        g_free(ctr_points_temp);
        fclose (pFile);
        return TRUE;
    }
    
err:
    setlocale(LC_NUMERIC, old_locale);
    if (pFile != NULL) fclose (pFile);
    if (ctr_points_temp != NULL) g_free(ctr_points_temp);
    return FALSE;
}

static void write_sharpblur(sharpblur_settings settings, GKeyFile* file) 
{
    gchar* group_name = "SHARPBLUR";
    
    g_key_file_set_integer(file, group_name, "amount", settings->amount);
}

static manipulation read_sharpblur(GKeyFile* file) 
{
    gchar* group_name = "SHARPBLUR";
    manipulation man = NULL;
    
    if (g_key_file_has_group(file, group_name)) {
        man = manipulation_sharpblur_new();
        sharpblur_settings settings = ((sharpblur_settings)man->settings);
        
        if (g_key_file_has_key(file, group_name, "amount", NULL)) 
            settings->amount = g_key_file_get_integer(file, group_name, "amount", NULL);
    }
    
    return man;
}

static void write_watermark(watermark_settings settings, GKeyFile* file) 
{
    gchar* group_name = "WATERMARK";
    
    g_key_file_set_boolean(file, group_name, "mode", settings->mode);
    g_key_file_set_string(file, group_name, "text", settings->text);
    g_key_file_set_string(file, group_name, "font", pango_font_description_to_string(settings->font));
    g_key_file_set_string(file, group_name, "color", gdk_color_to_string(&(settings->color)));
    if (settings->image_file != NULL) g_key_file_set_string(file, group_name, "image_file", settings->image_file);
    g_key_file_set_integer(file, group_name, "image_sizemode", settings->image_sizemode);
    g_key_file_set_double(file, group_name, "image_size_percent", settings->image_size_percent);
    g_key_file_set_integer(file, group_name, "opacity", settings->opacity);
    g_key_file_set_integer(file, group_name, "edge_distance", settings->edge_distance);
    g_key_file_set_integer(file, group_name, "position", settings->position);
}

static manipulation read_watermark(GKeyFile* file) 
{
    gchar* group_name = "WATERMARK";
    manipulation man = NULL;
    
    if (g_key_file_has_group(file, group_name)) {
        man = manipulation_watermark_new();
        watermark_settings settings = ((watermark_settings)man->settings);
        
        if (g_key_file_has_key(file, group_name, "mode", NULL)) 
            settings->mode = g_key_file_get_boolean(file, group_name, "mode", NULL);
            
        if (g_key_file_has_key(file, group_name, "text", NULL)) 
            settings->text = g_key_file_get_string(file, group_name, "text", NULL);
            
        if (g_key_file_has_key(file, group_name, "font", NULL)) 
            settings->font = pango_font_description_from_string(g_key_file_get_string(file, group_name, "font", NULL));
            
        if (g_key_file_has_key(file, group_name, "color", NULL)) 
            gdk_color_parse(g_key_file_get_string(file, group_name, "color", NULL), &(settings->color));
            
        if (g_key_file_has_key(file, group_name, "image_file", NULL)) 
            settings->image_file = g_key_file_get_string(file, group_name, "image_file", NULL);
            
        if (g_key_file_has_key(file, group_name, "image_sizemode", NULL)) 
            settings->image_sizemode = g_key_file_get_integer(file, group_name, "image_sizemode", NULL);
        
        if (g_key_file_has_key(file, group_name, "image_size_percent", NULL)) 
            settings->image_size_percent = g_key_file_get_double(file, group_name, "image_size_percent", NULL);
        
        if (g_key_file_has_key(file, group_name, "opacity", NULL)) 
            settings->opacity = g_key_file_get_integer(file, group_name, "opacity", NULL);
        
        if (g_key_file_has_key(file, group_name, "edge_distance", NULL)) 
            settings->edge_distance = g_key_file_get_integer(file, group_name, "edge_distance", NULL);
        
        if (g_key_file_has_key(file, group_name, "position", NULL)) 
            settings->position = g_key_file_get_integer(file, group_name, "position", NULL);
    }
    
    return man;
}

static void write_changeformat(changeformat_settings settings, GKeyFile* file) 
{
    gchar* group_name = "CHANGEFORMAT";
    
    g_key_file_set_integer(file, group_name, "format", settings->format);
    
    if(settings->format == FORMAT_GIF) {
        format_params_gif params = settings->params;
        g_key_file_set_boolean(file, group_name, "interlace", params->interlace);
    }
    else if(settings->format == FORMAT_JPEG) {
        format_params_jpeg params = settings->params;
        g_key_file_set_double(file, group_name, "quality", params->quality);
        g_key_file_set_double(file, group_name, "smoothing", params->smoothing);
        g_key_file_set_boolean(file, group_name, "entropy", params->entropy);
        g_key_file_set_boolean(file, group_name, "progressive", params->progressive);
        if (params->comment != NULL) g_key_file_set_string(file, group_name, "comment", params->comment);
        g_key_file_set_integer(file, group_name, "subsampling", params->subsampling);
        g_key_file_set_boolean(file, group_name, "baseline", params->baseline);
        g_key_file_set_integer(file, group_name, "markers", params->markers);
        g_key_file_set_integer(file, group_name, "dct", params->dct);
    }
    else if(settings->format == FORMAT_PNG) {
        format_params_png params = settings->params;
        g_key_file_set_boolean(file, group_name, "interlace", params->interlace);
        g_key_file_set_integer(file, group_name, "compression", params->compression);
        g_key_file_set_boolean(file, group_name, "savebgc", params->savebgc);
        g_key_file_set_boolean(file, group_name, "savegamma", params->savegamma);
        g_key_file_set_boolean(file, group_name, "saveoff", params->saveoff);
        g_key_file_set_boolean(file, group_name, "savephys", params->savephys);
        g_key_file_set_boolean(file, group_name, "savetime", params->savetime);
        g_key_file_set_boolean(file, group_name, "savecomm", params->savecomm);
        g_key_file_set_boolean(file, group_name, "savetrans", params->savetrans);
    }
    else if(settings->format == FORMAT_TGA) {
        format_params_tga params = settings->params;
        g_key_file_set_boolean(file, group_name, "rle", params->rle);
        g_key_file_set_integer(file, group_name, "origin", params->origin);
    }
    else if(settings->format == FORMAT_TIFF) {
        format_params_tiff params = settings->params;
        g_key_file_set_integer(file, group_name, "compression", params->compression);
    }
    else if(settings->format == FORMAT_HEIF) {
        format_params_heif params = settings->params;
        g_key_file_set_boolean(file, group_name, "lossless", params->lossless);
        g_key_file_set_integer(file, group_name, "quality", params->quality);
    }
    else if(settings->format == FORMAT_WEBP) {
        format_params_webp params = settings->params;
        g_key_file_set_integer(file, group_name, "preset", params->preset);
        g_key_file_set_boolean(file, group_name, "lossless", params->lossless);
        g_key_file_set_double(file, group_name, "quality", params->quality);
        g_key_file_set_double(file, group_name, "alpha_quality", params->alpha_quality);
        g_key_file_set_boolean(file, group_name, "animation", params->animation);
        g_key_file_set_boolean(file, group_name, "anim_loop", params->anim_loop);
        g_key_file_set_boolean(file, group_name, "minimize_size", params->minimize_size);
        g_key_file_set_integer(file, group_name, "kf_distance", params->kf_distance);
        g_key_file_set_boolean(file, group_name, "exif", params->exif);
        g_key_file_set_boolean(file, group_name, "iptc", params->iptc);
        g_key_file_set_boolean(file, group_name, "xmp", params->xmp);
        g_key_file_set_integer(file, group_name, "delay", params->delay);
        g_key_file_set_integer(file, group_name, "force_delay", params->force_delay);
    }
}

static manipulation read_changeformat(GKeyFile* file) 
{
    gchar* group_name = "CHANGEFORMAT";
    manipulation man = NULL;
    
    if (g_key_file_has_group(file, group_name)) {
        man = manipulation_changeformat_new();
        changeformat_settings settings = ((changeformat_settings)man->settings);
        
        if (g_key_file_has_key(file, group_name, "format", NULL)) {
            settings->format = g_key_file_get_integer(file, group_name, "format", NULL);
        
            if (settings->format == FORMAT_GIF) {
                settings->params = (format_params_gif) g_malloc(sizeof(struct changeformat_params_gif));
                format_params_gif params = settings->params;
                
                if (g_key_file_has_key(file, group_name, "interlace", NULL)) 
                    params->interlace = g_key_file_get_boolean(file, group_name, "interlace", NULL);
            }
            else if (settings->format == FORMAT_JPEG) {
                settings->params = (format_params_jpeg) g_malloc(sizeof(struct changeformat_params_jpeg));
                format_params_jpeg params = settings->params;
                
                if (g_key_file_has_key(file, group_name, "quality", NULL)) 
                    params->quality = g_key_file_get_double(file, group_name, "quality", NULL);
                
                if (g_key_file_has_key(file, group_name, "smoothing", NULL)) 
                    params->smoothing = g_key_file_get_double(file, group_name, "smoothing", NULL);
                
                if (g_key_file_has_key(file, group_name, "entropy", NULL)) 
                    params->entropy = g_key_file_get_boolean(file, group_name, "entropy", NULL);
                
                if (g_key_file_has_key(file, group_name, "progressive", NULL)) 
                    params->progressive = g_key_file_get_boolean(file, group_name, "progressive", NULL);
                
                if (g_key_file_has_key(file, group_name, "comment", NULL)) 
                    params->comment = g_key_file_get_string(file, group_name, "comment", NULL);
                    
                if (g_key_file_has_key(file, group_name, "subsampling", NULL)) 
                    params->subsampling = g_key_file_get_integer(file, group_name, "subsampling", NULL);
                    
                if (g_key_file_has_key(file, group_name, "baseline", NULL)) 
                    params->baseline = g_key_file_get_boolean(file, group_name, "baseline", NULL);
                
                if (g_key_file_has_key(file, group_name, "markers", NULL)) 
                    params->markers = g_key_file_get_integer(file, group_name, "markers", NULL);
                    
                if (g_key_file_has_key(file, group_name, "dct", NULL)) 
                    params->dct = g_key_file_get_integer(file, group_name, "dct", NULL);
            }
            else if (settings->format == FORMAT_PNG) {
                settings->params = (format_params_png) g_malloc(sizeof(struct changeformat_params_png));
                format_params_png params = settings->params;
                
                if (g_key_file_has_key(file, group_name, "interlace", NULL)) 
                    params->interlace = g_key_file_get_boolean(file, group_name, "interlace", NULL);
                
                if (g_key_file_has_key(file, group_name, "compression", NULL)) 
                    params->compression = g_key_file_get_integer(file, group_name, "compression", NULL);
                    
                if (g_key_file_has_key(file, group_name, "savebgc", NULL)) 
                    params->savebgc = g_key_file_get_boolean(file, group_name, "savebgc", NULL);
                    
                if (g_key_file_has_key(file, group_name, "savegamma", NULL)) 
                    params->savegamma = g_key_file_get_boolean(file, group_name, "savegamma", NULL);
                    
                if (g_key_file_has_key(file, group_name, "saveoff", NULL)) 
                    params->saveoff = g_key_file_get_boolean(file, group_name, "saveoff", NULL);
                    
                if (g_key_file_has_key(file, group_name, "savephys", NULL)) 
                    params->savephys = g_key_file_get_boolean(file, group_name, "savephys", NULL);
                    
                if (g_key_file_has_key(file, group_name, "savetime", NULL)) 
                    params->savetime = g_key_file_get_boolean(file, group_name, "savetime", NULL);
                    
                if (g_key_file_has_key(file, group_name, "savecomm", NULL)) 
                    params->savecomm = g_key_file_get_boolean(file, group_name, "savecomm", NULL);
                    
                if (g_key_file_has_key(file, group_name, "savetrans", NULL)) 
                    params->savetrans = g_key_file_get_boolean(file, group_name, "savetrans", NULL);
            }
            else if (settings->format == FORMAT_TGA) {
                settings->params = (format_params_tga) g_malloc(sizeof(struct changeformat_params_tga));
                format_params_tga params = settings->params;
                
                if (g_key_file_has_key(file, group_name, "rle", NULL)) 
                    params->rle = g_key_file_get_boolean(file, group_name, "rle", NULL);
                    
                if (g_key_file_has_key(file, group_name, "origin", NULL)) 
                    params->origin = g_key_file_get_integer(file, group_name, "origin", NULL);
            }
            else if (settings->format == FORMAT_TIFF) {
                settings->params = (format_params_tiff) g_malloc(sizeof(struct changeformat_params_tiff));
                format_params_tiff params = settings->params;
                
                if (g_key_file_has_key(file, group_name, "compression", NULL)) 
                    params->compression = g_key_file_get_integer(file, group_name, "compression", NULL);
            }
            else if (settings->format == FORMAT_HEIF) {
                settings->params = (format_params_heif) g_malloc(sizeof(struct changeformat_params_heif));
                format_params_heif params = settings->params;
                
                if (g_key_file_has_key(file, group_name, "lossless", NULL)) 
                    params->lossless = g_key_file_get_boolean(file, group_name, "lossless", NULL);
                    
                if (g_key_file_has_key(file, group_name, "quality", NULL)) 
                    params->quality = g_key_file_get_integer(file, group_name, "quality", NULL);
            }
            else if (settings->format == FORMAT_WEBP) {
                settings->params = (format_params_webp) g_malloc(sizeof(struct changeformat_params_webp));
                format_params_webp params = settings->params;
                
                if (g_key_file_has_key(file, group_name, "preset", NULL)) 
                    params->preset = g_key_file_get_integer(file, group_name, "preset", NULL);

                if (g_key_file_has_key(file, group_name, "lossless", NULL)) 
                    params->lossless = g_key_file_get_boolean(file, group_name, "lossless", NULL);
                    
                if (g_key_file_has_key(file, group_name, "quality", NULL)) 
                    params->quality = g_key_file_get_double(file, group_name, "quality", NULL);

                if (g_key_file_has_key(file, group_name, "alpha_quality", NULL)) 
                    params->alpha_quality = g_key_file_get_double(file, group_name, "alpha_quality", NULL);

                if (g_key_file_has_key(file, group_name, "animation", NULL)) 
                    params->animation = g_key_file_get_boolean(file, group_name, "animation", NULL);

                if (g_key_file_has_key(file, group_name, "anim_loop", NULL)) 
                    params->anim_loop = g_key_file_get_boolean(file, group_name, "anim_loop", NULL);

                if (g_key_file_has_key(file, group_name, "minimize_size", NULL)) 
                    params->minimize_size = g_key_file_get_boolean(file, group_name, "minimize_size", NULL);

                if (g_key_file_has_key(file, group_name, "kf_distance", NULL)) 
                    params->kf_distance = g_key_file_get_integer(file, group_name, "kf_distance", NULL);

                if (g_key_file_has_key(file, group_name, "exif", NULL)) 
                    params->exif = g_key_file_get_boolean(file, group_name, "exif", NULL);

                if (g_key_file_has_key(file, group_name, "iptc", NULL)) 
                    params->iptc = g_key_file_get_boolean(file, group_name, "iptc", NULL);

                if (g_key_file_has_key(file, group_name, "xmp", NULL)) 
                    params->xmp = g_key_file_get_boolean(file, group_name, "xmp", NULL);

                if (g_key_file_has_key(file, group_name, "delay", NULL)) 
                    params->delay = g_key_file_get_boolean(file, group_name, "delay", NULL);

                if (g_key_file_has_key(file, group_name, "force_delay", NULL)) 
                    params->force_delay = g_key_file_get_integer(file, group_name, "force_delay", NULL);
            }
        }
    }
    
    return man;
}

static void write_rename(rename_settings settings, GKeyFile* file) 
{
    gchar* group_name = "RENAME";
    
    if (settings->pattern != NULL) g_key_file_set_string(file, group_name, "pattern", settings->pattern);
}

static manipulation read_rename(GKeyFile* file) 
{
    gchar* group_name = "RENAME";
    manipulation man = NULL;
    
    if (g_key_file_has_group(file, group_name)) {
        man = manipulation_rename_new();
        rename_settings settings = ((rename_settings)man->settings);
        
        if (g_key_file_has_key(file, group_name, "pattern", NULL))
            settings->pattern = g_key_file_get_string(file, group_name, "pattern", NULL);
    }
    
    return man;
}

static void write_userdef(userdef_settings settings, GKeyFile* file, int id) 
{
    gchar* group_name = g_strdup_printf("USERDEF%d", id);
    
    g_key_file_set_string(file, group_name, "procedure", settings->procedure);
    g_key_file_set_integer(file, group_name, "num_params", settings->num_params);
    
    if (settings->num_params > 0) {        
        int param_i;
        GdkColor tempcolor;
        for (param_i = 0; param_i < settings->num_params; param_i++) {
            
            gchar* param_i_str = g_strdup_printf("PARAM%d", param_i);
            switch(settings->params[param_i].type) {
                case GIMP_PDB_INT32:
                    g_key_file_set_integer(file, group_name, param_i_str, settings->params[param_i].data.d_int32);
                    break;
                case GIMP_PDB_INT16:
                    g_key_file_set_integer(file, group_name, param_i_str, settings->params[param_i].data.d_int16);
                    break;
                case GIMP_PDB_INT8:
                    g_key_file_set_integer(file, group_name, param_i_str, settings->params[param_i].data.d_int8);
                    break;
                case GIMP_PDB_FLOAT: 
                    g_key_file_set_double(file, group_name, param_i_str, settings->params[param_i].data.d_float);
                    break;
                case GIMP_PDB_STRING: 
                    g_key_file_set_string(file, group_name, param_i_str, settings->params[param_i].data.d_string);
                    break;
                case GIMP_PDB_COLOR:
                    tempcolor.red = (guint16)(((settings->params[param_i]).data.d_color.r)*65535);
                    tempcolor.green = (guint16)(((settings->params[param_i]).data.d_color.g)*65535);
                    tempcolor.blue = (guint16)(((settings->params[param_i]).data.d_color.b)*65535);
                    
                    g_key_file_set_string(file, group_name, param_i_str, gdk_color_to_string(&(tempcolor)));
                    break;
                
                default: 
                    g_key_file_set_string(file, group_name, param_i_str, "NOT_USED");
                break;
            }
        }
    }
}

static manipulation read_userdef(GKeyFile* file, int id) 
{
    gchar* group_name = g_strdup_printf("USERDEF%d", id);
    manipulation man = NULL;
    
    if (g_key_file_has_group(file, group_name)) {
        man = manipulation_userdef_new();
        userdef_settings settings = ((userdef_settings)man->settings);
        
        if (g_key_file_has_key(file, group_name, "procedure", NULL) && g_key_file_has_key(file, group_name, "num_params", NULL)) {
            settings->procedure = g_key_file_get_string(file, group_name, "procedure", NULL);
            settings->num_params = g_key_file_get_integer(file, group_name, "num_params", NULL);
            
            settings->params = g_new(GimpParam, settings->num_params);
            
            int param_i;
            GimpParamDef param_info;
            GdkColor usercolor;
            GimpRGB rgbdata;
            for (param_i = 0; param_i < settings->num_params; param_i++) {
                param_info = pdb_proc_get_param_info(settings->procedure, param_i);
                
                settings->params[param_i].type = param_info.type;
                gchar* param_i_str = g_strdup_printf("PARAM%d", param_i);
                switch(settings->params[param_i].type) {
                    case GIMP_PDB_INT32:
                        (settings->params[param_i]).data.d_int32 = (gint32)g_key_file_get_integer(file, group_name, param_i_str, NULL);
                        break;
                        
                    case GIMP_PDB_INT16:
                        (settings->params[param_i]).data.d_int16 = (gint16)g_key_file_get_integer(file, group_name, param_i_str, NULL);
                        break;
                        
                    case GIMP_PDB_INT8:
                        (settings->params[param_i]).data.d_int8 = (gint8)g_key_file_get_integer(file, group_name, param_i_str, NULL);
                        break;
                        
                    case GIMP_PDB_FLOAT: 
                        (settings->params[param_i]).data.d_float = (gdouble)g_key_file_get_double(file, group_name, param_i_str, NULL);
                        break;
                        
                    case GIMP_PDB_STRING: 
                        (settings->params[param_i]).data.d_string = g_key_file_get_string(file, group_name, param_i_str, NULL);
                        break;
                    
                    case GIMP_PDB_COLOR: 
                        gdk_color_parse (g_key_file_get_string(file, group_name, param_i_str, NULL), &usercolor);
                        gimp_rgb_set(&rgbdata, (gdouble)usercolor.red/65535, (gdouble)usercolor.green/65535, (gdouble)usercolor.blue/65535);
                        (settings->params[param_i]).data.d_color = rgbdata;
                        break;
                        
                    default: break;
                }
            }
        }
    }
    
    return man;
}
