#ifndef __GUI_USERDEF_H__
#define __GUI_USERDEF_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define PROCLIST_W 210
#define PROCLIST_H 205

GtkWidget* bimp_userdef_gui_new(userdef_settings, GtkWidget*);
void bimp_userdef_save(userdef_settings);

#endif
