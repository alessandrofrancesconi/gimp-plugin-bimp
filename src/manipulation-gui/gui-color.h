#ifndef __GUI_COLOR_H__
#define __GUI_COLOR_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

GtkWidget* bimp_color_gui_new(color_settings);
void bimp_color_save(color_settings);

#endif
