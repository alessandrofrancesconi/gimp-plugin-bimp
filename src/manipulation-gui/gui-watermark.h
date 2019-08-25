#ifndef __GUI_WATERMARK_H__
#define __GUI_WATERMARK_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

GtkWidget* bimp_watermark_gui_new(watermark_settings);
void bimp_watermark_save(watermark_settings);

#endif
