#ifndef __GUI_CHANGEFORMAT_H__
#define __GUI_CHANGEFORMAT_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define CHANGEFORMAT_WINDOW_W 350
#define CHANGEFORMAT_WINDOW_H 265

#define COMBO_FORMAT_W CHANGEFORMAT_WINDOW_W - 50

#define TEXT_H 40
#define SCALE_H 35
#define LABEL_H 30
#define COMBO_H 30
#define SPIN_W 60
#define SPIN_H 25

#define SCALE_COMPRESSION_W COMBO_COMPRESSION_W

#define FRAME_PARAMS_W CHANGEFORMAT_WINDOW_W - 100
#define FRAME_PARAMS_H CHANGEFORMAT_WINDOW_H - COMBO_H - 115

GtkWidget* bimp_changeformat_gui_new(changeformat_settings, GtkWidget*);
void bimp_changeformat_save(changeformat_settings);

#endif
