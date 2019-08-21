#ifndef __GUI_CROP_H__
#define __GUI_CROP_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

GtkWidget* bimp_crop_gui_new(crop_settings);
void bimp_crop_save(crop_settings);

#endif
