#ifndef __GUI_USERDEF_H__
#define __GUI_USERDEF_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define USERDEF_WINDOW_W 550

#define SEARCH_W 120
#define SEARCH_H 25

#define LABEL_HELP_W USERDEF_WINDOW_W - 20
#define LABEL_HELP_H 50

#define PROCLIST_W 210
#define PROCLIST_H 205

#define PROCPARAM_W USERDEF_WINDOW_W - PROCLIST_W - 30
#define PROCPARAM_H PROCLIST_H + SEARCH_H

#define PARAM_WIDGET_W 90
#define PARAM_WIDGET_H 25

GtkWidget* bimp_userdef_gui_new(userdef_settings, GtkWidget*);
void bimp_userdef_save(userdef_settings);

#endif
