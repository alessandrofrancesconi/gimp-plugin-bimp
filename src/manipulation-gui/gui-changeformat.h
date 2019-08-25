#ifndef __GUI_CHANGEFORMAT_H__
#define __GUI_CHANGEFORMAT_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

GtkWidget* bimp_changeformat_gui_new(changeformat_settings, GtkWidget*);
void bimp_changeformat_save(changeformat_settings);

#endif
