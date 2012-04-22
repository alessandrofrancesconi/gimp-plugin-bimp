#ifndef __GUI_WATERMARK_H__
#define __GUI_WATERMARK_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define WATERMARK_WINDOW_W 320
#define WATERMARK_WINDOW_H 450

#define BUTTON_W WATERMARK_WINDOW_W - 80
#define BUTTON_H 30

#define LABEL_TRANSP_W 80
#define LABEL_TRANSP_H 40

#define LABEL_PERCENT_W 30
#define LABEL_PERCENT_H 40

#define SCALE_TRANSP_W WATERMARK_WINDOW_W - LABEL_TRANSP_W - LABEL_PERCENT_W - 40
#define SCALE_TRANSP_H LABEL_TRANSP_H

GtkWidget* bimp_watermark_gui_new(watermark_settings);
void bimp_watermark_save(watermark_settings);

#endif
