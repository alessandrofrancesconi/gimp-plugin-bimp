#ifndef __GUI_COLOR_H__
#define __GUI_COLOR_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define COLOR_WINDOW_W 310

#define LABEL_BC_W 80
#define LABEL_BC_H 40

#define SCALE_BC_W COLOR_WINDOW_W - LABEL_BC_W - 40
#define SCALE_BC_H LABEL_BC_H

#define INPUT_W 200
#define INPUT_H 30

GtkWidget* bimp_color_gui_new(color_settings);
void bimp_color_save(color_settings);

#endif
