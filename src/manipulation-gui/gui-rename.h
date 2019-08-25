#ifndef __GUI_RENAME_H__
#define __GUI_RENAME_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

GtkWidget* bimp_rename_gui_new(rename_settings, GtkWidget*);
void bimp_rename_save(rename_settings);

#endif
