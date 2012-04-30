#ifndef __GUI_USERDEF_H__
#define __GUI_USERDEF_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define USERDEF_WINDOW_W 420
#define USERDEF_WINDOW_H 400

#define LABEL_HELP_W USERDEF_WINDOW_W - 20
#define LABEL_HELP_H 50

#define BUTTON_CHOOSE_W 100
#define BUTTON_CHOOSE_H 30

#define FRAME_PARAM_W USERDEF_WINDOW_W - 20
#define FRAME_PARAM_H USERDEF_WINDOW_H - LABEL_HELP_H - BUTTON_CHOOSE_H - 110

#define PARAM_WIDGET_W 125
#define PARAM_WIDGET_H 25

GtkWidget* bimp_userdef_gui_new(userdef_settings, GtkWidget*);
void bimp_userdef_save(userdef_settings);

#endif
