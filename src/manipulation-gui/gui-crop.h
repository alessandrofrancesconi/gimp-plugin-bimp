#ifndef __GUI_CROP_H__
#define __GUI_CROP_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define LABEL_CROP_W 85
#define LABEL_CROP_H 25

GtkWidget* bimp_crop_gui_new(crop_settings);
void bimp_crop_save(crop_settings);

#endif
