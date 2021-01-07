/*
 * Functions to initialize and manage manipulations
 */

#include <string.h>
#include <gtk/gtk.h>
#include "bimp-manipulations.h"
#include "bimp-manipulations-gui.h"
#include "plugin-intl.h"


GSList* bimp_selected_manipulations;

/* Appends a default manipulation to the step list */
manipulation bimp_append_manipulation(manipulation_type type)
{
    manipulation newman = NULL; /* newman, paul newman. */
    
    if (type != MANIP_USERDEF && bimp_list_contains_manip(type)) {
        return NULL;
    }
    else {
        if (type == MANIP_RESIZE) {
            newman = manipulation_resize_new();
        }
        else if (type == MANIP_CROP) {
            newman = manipulation_crop_new();
        }
        else if (type == MANIP_FLIPROTATE) {
            newman = manipulation_fliprotate_new();
        }
        else if (type == MANIP_COLOR) {
            newman = manipulation_color_new();
        }
        else if (type == MANIP_SHARPBLUR) {
            newman = manipulation_sharpblur_new();
        }
        else if (type == MANIP_WATERMARK) {
            newman = manipulation_watermark_new();
        }
        else if (type == MANIP_CHANGEFORMAT) {
            newman = manipulation_changeformat_new();
        }
        else if (type == MANIP_RENAME) {
            newman = manipulation_rename_new();
        }
        else if (type == MANIP_USERDEF) {
            newman = manipulation_userdef_new();
        }
                
        bimp_selected_manipulations = g_slist_append(bimp_selected_manipulations, newman);
        
        return newman;
    }
}

/* remove a manipulation from the list */
void bimp_remove_manipulation(manipulation man)
{
    bimp_selected_manipulations = g_slist_remove(bimp_selected_manipulations, man);
}

/* check if at least one manipulation of type 'search' has been added to the list */
gboolean bimp_list_contains_manip(manipulation_type search) 
{
    gboolean find = FALSE;
    GSList *iterator = NULL;
    manipulation_type elem_type;
    
    for (iterator = bimp_selected_manipulations; iterator && find == FALSE; iterator = iterator->next) {
        elem_type = ((manipulation)(iterator->data))->type;
        if (elem_type == search) find = TRUE;
    }

    return find;
}

gboolean bimp_list_contains_savingplugin() 
{
    gboolean found = FALSE;
    GSList *iterator = NULL;
    manipulation man;
    
    for (iterator = bimp_selected_manipulations; iterator && found == FALSE; iterator = iterator->next) {
        man = (manipulation)(iterator->data);
        if (man->type == MANIP_USERDEF && strstr(((userdef_settings)(man->settings))->procedure, "-save") != NULL) found = TRUE;
    }

    return found;
}

/* get a subset of the manipulation list composed by the selected saving plugins (for example file-dds-save) 
gboolean bimp_list_get_savingprocedures() 
{
    GSList *sublist;
    GSList *iterator = NULL;
    manipulation_type elem_type;
    
    // TODO

    return sublist;
}*/

/* get the first occurrence of manipulation of type 'search' from the selected ones */
manipulation bimp_list_get_manip(manipulation_type search) 
{
    gboolean find = FALSE;
    manipulation found_man = NULL;
    GSList *iterator = NULL;
    
    for (iterator = bimp_selected_manipulations; iterator && find == FALSE; iterator = iterator->next) {
        found_man = (manipulation)(iterator->data);
        if (found_man->type == search) find = TRUE;
    }
    
    return found_man;
}

/* set of constructors for each type of manipulation (with default values) */

manipulation manipulation_resize_new() 
{
    manipulation resize;
    resize = (manipulation) g_malloc(sizeof(struct manip_str));
    resize->type = MANIP_RESIZE;
    resize->icon = "/gimp/plugin/bimp/icons/stock-resize.png";
    resize->settings = (resize_settings) g_malloc(sizeof(struct manip_resize_set));
    ((resize_settings)resize->settings)->new_w_pc = 100.0;
    ((resize_settings)resize->settings)->new_h_pc = 100.0;
    ((resize_settings)resize->settings)->new_w_px = 640;
    ((resize_settings)resize->settings)->new_h_px = 480;
    ((resize_settings)resize->settings)->resize_mode_width = RESIZE_PERCENT;
    ((resize_settings)resize->settings)->resize_mode_height = RESIZE_PERCENT;
    ((resize_settings)resize->settings)->stretch_mode = STRETCH_ALLOW;
    gdk_color_parse("black", &(((resize_settings)resize->settings)->padding_color));
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &(((resize_settings)resize->settings)->padding_color), TRUE, TRUE);
    ((resize_settings)resize->settings)->padding_color_alpha = G_MAXUINT16;
    ((resize_settings)resize->settings)->interpolation = GIMP_INTERPOLATION_CUBIC;
    ((resize_settings)resize->settings)->change_res = FALSE;
    ((resize_settings)resize->settings)->new_res_x = 72.000;
    ((resize_settings)resize->settings)->new_res_y = 72.000;
    
    return resize;
}

manipulation manipulation_crop_new() 
{
    manipulation crop;
    crop = (manipulation) g_malloc(sizeof(struct manip_str));
    crop->type = MANIP_CROP;
    crop->icon = "/gimp/plugin/bimp/icons/stock-crop.png";
    crop->settings = (crop_settings) g_malloc(sizeof(struct manip_crop_set));
    ((crop_settings)crop->settings)->new_w = 640;
    ((crop_settings)crop->settings)->new_h = 480;
    ((crop_settings)crop->settings)->manual = FALSE;
    ((crop_settings)crop->settings)->ratio = CROP_PRESET_11;
    ((crop_settings)crop->settings)->custom_ratio1 = 1.0;
    ((crop_settings)crop->settings)->custom_ratio2 = 1.0;
    ((crop_settings)crop->settings)->start_pos = CROP_START_CC;
    
    return crop;
}

manipulation manipulation_fliprotate_new() 
{
    manipulation fliprotate;
    fliprotate = (manipulation) g_malloc(sizeof(struct manip_str));
    fliprotate->type = MANIP_FLIPROTATE;
    fliprotate->icon = "/gimp/plugin/bimp/icons/stock-rotate.png";
    fliprotate->settings = (fliprotate_settings) g_malloc(sizeof(struct manip_fliprotate_set));
    ((fliprotate_settings)fliprotate->settings)->flip_h = FALSE;
    ((fliprotate_settings)fliprotate->settings)->flip_v = FALSE;
    ((fliprotate_settings)fliprotate->settings)->rotate = FALSE;
    ((fliprotate_settings)fliprotate->settings)->rotation_type = GIMP_ROTATE_90;
    
    return fliprotate;
}

manipulation manipulation_color_new() 
{
    manipulation color;
    color = (manipulation) g_malloc(sizeof(struct manip_str));
    color->type = MANIP_COLOR;
    color->icon = "/gimp/plugin/bimp/icons/stock-color.png";
    color->settings = (color_settings) g_malloc(sizeof(struct manip_color_set));
    ((color_settings)color->settings)->brightness = 0;
    ((color_settings)color->settings)->contrast = 0;
    ((color_settings)color->settings)->grayscale = FALSE;
    ((color_settings)color->settings)->levels_auto = FALSE;
    ((color_settings)color->settings)->curve_file = NULL;
    
    return color;
}

manipulation manipulation_sharpblur_new() 
{
    manipulation sharpblur;
    sharpblur = (manipulation) g_malloc(sizeof(struct manip_str));
    sharpblur->type = MANIP_SHARPBLUR;
    sharpblur->icon = "/gimp/plugin/bimp/icons/stock-sharp.png";
    sharpblur->settings = (sharpblur_settings) g_malloc(sizeof(struct manip_sharpblur_set));
    ((sharpblur_settings)sharpblur->settings)->amount = 0;
    
    return sharpblur;
}

manipulation manipulation_watermark_new() 
{
    manipulation watermark;
    watermark = (manipulation) g_malloc(sizeof(struct manip_str));
    watermark->type = MANIP_WATERMARK;
    watermark->icon = "/gimp/plugin/bimp/icons/stock-watermark.png";
    watermark->settings = (watermark_settings) g_malloc(sizeof(struct manip_watermark_set));
    ((watermark_settings)watermark->settings)->mode = TRUE;
    ((watermark_settings)watermark->settings)->text = "";
    ((watermark_settings)watermark->settings)->font = pango_font_description_copy(pango_font_description_from_string("Sans 16px"));
    gdk_color_parse("black", &(((watermark_settings)watermark->settings)->color));
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &(((watermark_settings)watermark->settings)->color), TRUE, TRUE);
    ((watermark_settings)watermark->settings)->image_file = NULL;
    ((watermark_settings)watermark->settings)->image_sizemode = WM_IMG_NOSIZE;
    ((watermark_settings)watermark->settings)->image_size_percent = 25.0;
    ((watermark_settings)watermark->settings)->opacity = 100.0;
    ((watermark_settings)watermark->settings)->edge_distance = 0;
    ((watermark_settings)watermark->settings)->position = WM_POS_BR;
    
    return watermark;
}

manipulation manipulation_changeformat_new() 
{
    manipulation changeformat;
    changeformat = (manipulation) g_malloc(sizeof(struct manip_str));
    changeformat->type = MANIP_CHANGEFORMAT;
    changeformat->icon = "/gimp/plugin/bimp/icons/stock-changeformat.png";
    changeformat->settings = (changeformat_settings) g_malloc(sizeof(struct manip_changeformat_set));
    ((changeformat_settings)changeformat->settings)->format = FORMAT_JPEG;
    ((changeformat_settings)changeformat->settings)->params = (format_params_jpeg) g_malloc(sizeof(struct changeformat_params_jpeg));
    ((format_params_jpeg)((changeformat_settings)changeformat->settings)->params)->quality = 85.0;
    ((format_params_jpeg)((changeformat_settings)changeformat->settings)->params)->smoothing = 0.0;
    ((format_params_jpeg)((changeformat_settings)changeformat->settings)->params)->entropy = TRUE;
    ((format_params_jpeg)((changeformat_settings)changeformat->settings)->params)->progressive = FALSE;
    ((format_params_jpeg)((changeformat_settings)changeformat->settings)->params)->comment = "";
    ((format_params_jpeg)((changeformat_settings)changeformat->settings)->params)->subsampling = 2;
    ((format_params_jpeg)((changeformat_settings)changeformat->settings)->params)->baseline = TRUE;
    ((format_params_jpeg)((changeformat_settings)changeformat->settings)->params)->markers = 0;
    ((format_params_jpeg)((changeformat_settings)changeformat->settings)->params)->dct = 1;
    
    return changeformat;
}

manipulation manipulation_rename_new() 
{
    manipulation rename;
    rename = (manipulation) g_malloc(sizeof(struct manip_str));
    rename->type = MANIP_RENAME;
    rename->icon = "/gimp/plugin/bimp/icons/stock-rename.png";
    rename->settings = (rename_settings) g_malloc(sizeof(struct manip_rename_set));
    ((rename_settings)rename->settings)->pattern = RENAME_KEY_ORIG;
    
    return rename;
}

manipulation manipulation_userdef_new() 
{
    manipulation userdef;
    userdef = (manipulation) g_malloc(sizeof(struct manip_str));
    userdef->type = MANIP_USERDEF;
    userdef->icon = "/gimp/plugin/bimp/icons/stock-userdef.png";
    userdef->settings = (userdef_settings) g_malloc(sizeof(struct manip_userdef_set));
    ((userdef_settings)userdef->settings)->procedure = NULL;
    ((userdef_settings)userdef->settings)->num_params = 0;
    ((userdef_settings)userdef->settings)->params = NULL;
    
    return userdef;
}

char* bimp_manip_get_string (manipulation_type type) 
{
    char* man_string = "";
    
    if (type == MANIP_RESIZE) {
        man_string = _("Resize");
    }
    else if (type == MANIP_CROP) {
        man_string = _("Crop");
    }
    else if (type == MANIP_FLIPROTATE) {
        man_string = _("Flip or Rotate");
    }
    else if (type == MANIP_COLOR) {
        man_string = _("Color correction");
    }
    else if (type == MANIP_SHARPBLUR) {
        man_string = _("Sharp or Blur");
    }
    else if (type == MANIP_WATERMARK) {
        man_string = _("Add a Watermark");
    }
    else if (type == MANIP_CHANGEFORMAT) {
        man_string = _("Change format and compression");
    }
    else if (type == MANIP_RENAME) {
        man_string = _("Rename with a pattern");
    }
    else if (type == MANIP_USERDEF) {
        man_string = _("Other GIMP procedure...");
    }
    
    return man_string;
}
