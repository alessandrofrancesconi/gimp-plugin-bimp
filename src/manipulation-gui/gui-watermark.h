#ifndef __GUI_WATERMARK_H__
#define __GUI_WATERMARK_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define WATERMARK_WINDOW_W 320

#define INPUT_W 200
#define INPUT_H 30

#define LABEL_W 60
#define LABEL_H 30

#define LABEL_TRANSP_W 80
#define LABEL_TRANSP_H 40

#define LABEL_PERCENT_W 30
#define LABEL_PERCENT_H 40

#define SCALE_TRANSP_W WATERMARK_WINDOW_W - LABEL_TRANSP_W - LABEL_PERCENT_W - 40
#define SCALE_TRANSP_H LABEL_TRANSP_H

#define FRAME_POSITION_W 140
#define FRAME_POSITION_H 120

#define BUTTON_POSITION_W 40
#define BUTTON_POSITION_H 30

GtkWidget* bimp_watermark_gui_new(watermark_settings);
void bimp_watermark_save(watermark_settings);

#endif
