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
	MANIP_SHARPEN,
	MANIP_WATERMARK,
	MANIP_CHANGEFORMAT,
	MANIP_RENAME,
	MANIP_USERDEF,
	MANIP_END
} manipulation_type;
static const char* manipulation_type_string[] = {
	"Resize",							/* MANIP_RESIZE */
	"Crop",								/* MANIP_CROP */
	"Flip or Rotate",					/* MANIP_FLIPROTATE */
	"Color correction",					/* MANIP_COLOR */
	"Sharpen",							/* MANIP_SHARPEN */
	"Apply a watermark text or image",	/* MANIP_WATERMARK */
	"Change format and quality",		/* MANIP_CHANGEFORMAT */
	"Rename with a pattern",			/* MANIP_RENAME */
	"Custom GIMP procedure"				/* MANIP_USERDEF */
};

typedef enum resize_mode {
	RESIZE_PERCENT = 0,
	RESIZE_PIXEL_BOTH,
	RESIZE_PIXEL_WIDTH,
	RESIZE_PIXEL_HEIGHT,
	RESIZE_END
} resize_mode;

typedef enum crop_preset {
	CROP_PRESET_11 = 0,
	CROP_PRESET_32,
	CROP_PRESET_43,
	CROP_PRESET_169,
	CROP_PRESET_1610,
	CROP_PRESET_EUPORT,
	CROP_PRESET_IPHONE,
	CROP_PRESET_IPAD,
	CROP_PRESET_END
} crop_preset;
static const char* crop_preset_string[] = {
	"One-to-one (1:1)",					/* CROP_PRESET_11 */
	"Classic 35 mm film (3:2)",			/* CROP_PRESET_32 */
	"Standard VGA monitor (4:3)",		/* CROP_PRESET_43 */
	"Widescreen (16:9)",				/* CROP_PRESET_169 */
	"Widescreen extended (16:10)",		/* CROP_PRESET_1610 */
	"EU Passport portrait (7:9)",		/* CROP_PRESET_EUPORT */
	"Standard smartphone screen (2:3)",	/* CROP_PRESET_IPHONE */
	"Standard tablet screen (3:4)"		/* CROP_PRESET_IPAD */
};
static const short int crop_preset_ratio[][2] = {
	{1,1},		/* CROP_PRESET_11 */
	{3,2},		/* CROP_PRESET_32 */
	{4,3},		/* CROP_PRESET_43 */
	{16,9},		/* CROP_PRESET_169 */
	{16,10},	/* CROP_PRESET_1610 */
	{7,9},		/* CROP_PRESET_EUPORT */
	{2,3},		/* CROP_PRESET_IPHONE */
	{3,4}		/* CROP_PRESET_IPAD */
};

typedef enum format_type {
	FORMAT_BMP = 0,
	FORMAT_GIF,
	FORMAT_ICON,
	FORMAT_JPEG,
	FORMAT_PNG,
	//FORMAT_RAW, // TODO
	FORMAT_TGA,
	FORMAT_TIFF,
	// XCF // TODO
	FORMAT_END
} format_type;
static const char* format_type_string[][2] = {
	{"bmp", "Bitmap (.bmp)"},							/* FORMAT_BMP */
	{"gif", "Gif (.gif)"},								/* FORMAT_GIF */
	{"ico", "Icon (.ico)"},								/* FORMAT_ICON */
	{"jpg", "Jpeg (.jpeg)"},							/* FORMAT_JPEG */
	{"png", "Portable Network Graphics (.png)"},		/* FORMAT_PNG */
	//{"raw", "Raw image data (.raw)"},					/* FORMAT_RAW */
	{"tga", "Targa (.tga)"},							/* FORMAT_TGA */
	{"tiff", "Tagged Image File Format (.tiff)"}		/* FORMAT_TIFF */
};

typedef enum watermark_position {
	WM_POS_TL = 0,
	WM_POS_TR,
	WM_POS_CC,
	WM_POS_BL,
	WM_POS_BR,
	WM_POS_END
} watermark_position;
static const char* watermark_position_string[] = {
	"Top-left",		/* WM_POS_TL */
	"Top-right",	/* WM_POS_TR */
	"Center",		/* WM_POS_CC */
	"Bottom-left",	/* WM_POS_BL */
	"Bottom-right"	/* WM_POS_BR */
};

typedef void *manipulation_settings;
typedef void *format_params;

/* Single manipulation struct */
typedef struct manip_str {
	manipulation_type type;
	GdkPixdata* icon;
	manipulation_settings settings; /* Pointer to one of the following settings structs */
} *manipulation;

/* Settings structs */

typedef struct manip_resize_set {
	gdouble newWpc;
	gdouble newHpc;
	gint newWpx;
	gint newHpx;
	resize_mode sizemode;
	gboolean aspect_ratio;
	GimpInterpolationType interpolation;
} *resize_settings;

typedef struct manip_crop_set {
	gint newW;
	gint newH;
	gboolean manual;
	crop_preset ratio;
} *crop_settings;

typedef struct manip_fliprotate_set {
	gboolean flipH;
	gboolean flipV;
	gboolean rotate;
	GimpRotationType rotate_type;
} *fliprotate_settings;

typedef struct manip_color_set {
	gint brightness;
	gint contrast;
	gboolean levels_auto;
	gboolean grayscale;
} *color_settings;

typedef struct manip_sharp_set { 
	int amount;
} *sharp_settings;

typedef struct manip_watermark_set {
	gboolean textmode; /* TRUE = text mode; FALSE = image mode */
	gchar* text;
	PangoFontDescription* font;
	GdkColor color;
	char* imagefile;
	float opacity;
	watermark_position position;
} *watermark_settings;

typedef struct manip_changeformat_set {
	format_type format;
	format_params params; /* Pointer to one of the format params structs */
} *changeformat_settings;

typedef struct changeformat_params_jpeg {
	float quality;
} *format_params_jpeg;

typedef struct changeformat_params_png {
	gboolean interlace;
	int compression;
} *format_params_png;

typedef struct changeformat_params_tga {
	gboolean rle;
} *format_params_tga;

typedef struct changeformat_params_tiff {
	int compression;
} *format_params_tiff;

typedef struct manip_rename_set {
	gchar* pattern;
} *rename_settings;

typedef struct manip_userdef_set {
	gchar* procedure;
	gint num_params;
	GimpParam* params; /* array of procedure params (GimpParamDef structs) */
	gchar** params_names; /* array of param names (mapped to GimpParam* params) */
} *userdef_settings;

manipulation bimp_append_manipulation(manipulation_type);
void bimp_remove_manipulation(manipulation);
gboolean bimp_list_contains_manip(manipulation_type);
manipulation bimp_list_get_manip(manipulation_type);

GSList* bimp_selected_manipulations; /* Manipulations selected by user */

#endif
