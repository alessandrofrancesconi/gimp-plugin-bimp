#ifndef __GUI_FLIPROTATE_H__
#define __GUI_FLIPROTATE_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define BUTTON_FLIP_W 100
#define BUTTON_FLIP_H 55

GtkWidget* bimp_fliprotate_gui_new(fliprotate_settings);
void bimp_fliprotate_save(fliprotate_settings);

#endif
