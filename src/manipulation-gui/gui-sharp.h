#ifndef __GUI_SHARP_H__
#define __GUI_SHARP_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define SHARP_WINDOW_W 300
#define SHARP_WINDOW_H 150

#define LABEL_AMOUNT_W 60
#define LABEL_AMOUNT_H 40

#define LABEL_PERCENT_W 30
#define LABEL_PERCENT_H 40

#define SCALE_AMOUNT_W SHARP_WINDOW_W - LABEL_AMOUNT_W - LABEL_PERCENT_W - 40
#define SCALE_AMOUNT_H LABEL_AMOUNT_H

GtkWidget* bimp_sharp_gui_new(sharp_settings);
void bimp_sharp_save(sharp_settings);

#endif
