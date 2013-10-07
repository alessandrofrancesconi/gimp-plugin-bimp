#ifndef __GUI_SHARPBLUR_H__
#define __GUI_SHARPBLUR_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define SHARPBLUR_WINDOW_W 400

#define LABEL_SCALE_W 110
#define LABEL_SCALE_H 60

#define SCALE_AMOUNT_W SHARPBLUR_WINDOW_W - (LABEL_SCALE_W*2) - 40
#define SCALE_AMOUNT_H LABEL_SCALE_H

GtkWidget* bimp_sharpblur_gui_new(sharpblur_settings);
void bimp_sharpblur_save(sharpblur_settings);

#endif
