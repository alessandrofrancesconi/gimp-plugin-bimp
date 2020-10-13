#ifndef __BIMP_MANIPULATIONS_H__
#define __BIMP_MANIPULATIONS_H__

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixdata.h>
#include <libgimp/gimp.h>

#define RENAME_KEY_ORIG "$$"
#define RENAME_KEY_COUNT "##"
#define RENAME_KEY_DATETIME "@@"

typedef enum manipulation_type {
    MANIP_RESIZE = 0,
    MANIP_CROP,
    MANIP_FLIPROTATE,
    MANIP_COLOR,
    MANIP_SHARPBLUR,
    MANIP_WATERMARK,
    MANIP_CHANGEFORMAT,
    MANIP_RENAME,
    MANIP_USERDEF,
    MANIP_END
} manipulation_type;

typedef enum resize_mode {
    RESIZE_PERCENT = 0,
    RESIZE_PIXEL,
    RESIZE_DISABLE,
    RESIZE_END
} resize_mode;

typedef enum stretch_mode {
    STRETCH_ALLOW = 0,
    STRETCH_ASPECT,
    STRETCH_PADDED,
    STRETCH_END
} stretch_mode;

typedef enum crop_preset {
    CROP_PRESET_11 = 0,
    CROP_PRESET_32,
    CROP_PRESET_43,
    CROP_PRESET_169,
    CROP_PRESET_1610,
    CROP_PRESET_EUPORT,
    CROP_PRESET_PHONE,
    CROP_PRESET_TALLPHONE,
    CROP_PRESET_TABLET,
    CROP_PRESET_CUSTOM,
    CROP_PRESET_END
} crop_preset;
static const short int crop_preset_ratio[][2] = {
    {1,1},        /* CROP_PRESET_11 */
    {3,2},        /* CROP_PRESET_32 */
    {4,3},        /* CROP_PRESET_43 */
    {16,9},        /* CROP_PRESET_169 */
    {16,10},    /* CROP_PRESET_1610 */
    {7,9},        /* CROP_PRESET_EUPORT */
    {2,3},        /* CROP_PRESET_PHONE */
    {40,71},    /* CROP_PRESET_TALLPHONE */
    {3,4}        /* CROP_PRESET_TABLET */
};

typedef enum crop_start_position {
    CROP_START_CC = 0,
    CROP_START_TL,
    CROP_START_TR,
    CROP_START_BL,
    CROP_START_BR,
    CROP_START_XY,
    CROP_START_END
} crop_start_position;

typedef enum format_type {
    FORMAT_BMP = 0,
    FORMAT_GIF,
    FORMAT_ICON,
    FORMAT_JPEG,
    FORMAT_PNG,
    FORMAT_TGA,
    FORMAT_TIFF,
    FORMAT_HEIF,
    FORMAT_WEBP,
    FORMAT_EXR,
    FORMAT_END
} format_type;
static const char* format_type_string[][2] = {
    {"bmp", "Bitmap (.bmp)"},                           /* FORMAT_BMP */
    {"gif", "Gif (.gif)"},                              /* FORMAT_GIF */
    {"ico", "Icon (.ico)"},                             /* FORMAT_ICON */
    {"jpg", "Jpeg (.jpeg)"},                            /* FORMAT_JPEG */
    {"png", "Portable Network Graphics (.png)"},        /* FORMAT_PNG */
    {"tga", "Targa (.tga)"},                            /* FORMAT_TGA */
    {"tiff", "Tagged Image File Format (.tiff)"},       /* FORMAT_TIFF */
    {"heif", "Heif (.heif)"},                           /* FORMAT_HEIF */
    {"webp", "WebP (.webp)"},                           /* FORMAT_WEBP */
    {"exr", "OpenEXR (.exr)"}                           /* FORMAT_EXR */
};

// First two bits = column, second two bits = row
typedef enum watermark_position {
    WM_POS_TL = 0,
    WM_POS_TC,
    WM_POS_TR,
    WM_POS_CL,
    WM_POS_CC,
    WM_POS_CR,
    WM_POS_BL,
    WM_POS_BC,
    WM_POS_BR,
    WM_POS_END
} watermark_position;

typedef enum watermark_image_sizemode {
    WM_IMG_NOSIZE = 0,
    WM_IMG_SIZEW,
    WM_IMG_SIZEH,
    WM_IMG_END
} watermark_image_sizemode;

typedef void *manipulation_settings;
typedef void *format_params;

/* Single manipulation struct */
typedef struct manip_str {
    manipulation_type type;
    char* icon;
    manipulation_settings settings; /* Pointer to one of the following settings structs */
} *manipulation;

/* Settings structs */

typedef struct manip_resize_set {
    gdouble new_w_pc;
    gdouble new_h_pc;
    gint new_w_px;
    gint new_h_px;
    resize_mode resize_mode_width;
    resize_mode resize_mode_height;
    stretch_mode stretch_mode;
    GdkColor padding_color;
    guint16 padding_color_alpha;
    GimpInterpolationType interpolation;
    gboolean change_res;
    gdouble new_res_x;
    gdouble new_res_y;
} *resize_settings;

typedef struct manip_crop_set {
    gint new_w;
    gint new_h;
    gboolean manual;
    crop_preset ratio;
    float custom_ratio1;
    float custom_ratio2;
    crop_start_position start_pos;
} *crop_settings;

typedef struct manip_fliprotate_set {
    gboolean flip_h;
    gboolean flip_v;
    gboolean rotate;
    GimpRotationType rotation_type;
} *fliprotate_settings;

typedef struct manip_color_set {
    gdouble brightness;
    gdouble contrast;
    gboolean levels_auto;
    gboolean grayscale;
    char* curve_file;
} *color_settings;

typedef struct manip_sharpblur_set { 
    int amount;
} *sharpblur_settings;

typedef struct manip_watermark_set {
    gboolean mode; /* TRUE = text mode; FALSE = image mode */
    gchar* text;
    PangoFontDescription* font;
    GdkColor color;
    char* image_file;
    watermark_image_sizemode image_sizemode;
    float image_size_percent;
    float opacity;
    int edge_distance;
    watermark_position position;
} *watermark_settings;

typedef struct manip_changeformat_set {
    format_type format;
    format_params params; /* Pointer to one of the format params structs */
} *changeformat_settings;

typedef struct changeformat_params_gif {
    gboolean interlace;
} *format_params_gif;

typedef struct changeformat_params_jpeg {
    float quality;
    float smoothing;
    gboolean entropy;
    gboolean progressive;
    gchar* comment;
    int subsampling;
    gboolean baseline;
    int markers;
    int dct;
} *format_params_jpeg;

typedef struct changeformat_params_png {
    gboolean interlace;
    int compression;
    gboolean savebgc;
    gboolean savegamma;
    gboolean saveoff;
    gboolean savephys;
    gboolean savetime;
    gboolean savecomm;
    gboolean savetrans;
} *format_params_png;

typedef struct changeformat_params_tga {
    gboolean rle;
    int origin;
} *format_params_tga;

typedef struct changeformat_params_tiff {
    int compression;
} *format_params_tiff;

typedef struct changeformat_params_heif {
    gboolean lossless;
    int quality;
} *format_params_heif;

typedef struct changeformat_params_webp {
    int preset;
    gboolean lossless;
    float quality;
    float alpha_quality;
    gboolean animation;
    gboolean anim_loop;
    gboolean minimize_size;
    int kf_distance;
    gboolean exif;
    gboolean iptc;
    gboolean xmp;
    int delay;
    int force_delay;
} *format_params_webp;

typedef struct manip_rename_set {
    gchar* pattern;
} *rename_settings;

typedef struct manip_userdef_set {
    gchar* procedure;
    gint num_params;
    GimpParam* params; /* array of procedure params (GimpParamDef structs) */
} *userdef_settings;

manipulation bimp_append_manipulation(manipulation_type);
void bimp_remove_manipulation(manipulation);
gboolean bimp_list_contains_manip(manipulation_type);
gboolean bimp_list_contains_savingplugin(void);
manipulation bimp_list_get_manip(manipulation_type);
char* bimp_manip_get_string(manipulation_type);

manipulation manipulation_sharpblur_new(void);
manipulation manipulation_resize_new(void);
manipulation manipulation_crop_new(void);
manipulation manipulation_fliprotate_new(void);
manipulation manipulation_color_new(void);
manipulation manipulation_watermark_new(void); 
manipulation manipulation_changeformat_new(void); 
manipulation manipulation_rename_new(void); 
manipulation manipulation_userdef_new(void); 

extern GSList* bimp_selected_manipulations; /* Manipulations selected by user */

#endif
