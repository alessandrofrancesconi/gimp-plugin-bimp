#ifndef __GUI_RESIZE_H__
#define __GUI_RESIZE_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define RESIZE_WINDOW_W 360
#define RESIZE_WINDOW_H 400

GtkWidget* bimp_resize_gui_new(resize_settings);
void bimp_resize_save(resize_settings);

#endif
