#ifndef __GUI_SHARPBLUR_H__
#define __GUI_SHARPBLUR_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define SCALE_AMOUNT_W 220

GtkWidget* bimp_sharpblur_gui_new(sharpblur_settings);
void bimp_sharpblur_save(sharpblur_settings);

#endif
