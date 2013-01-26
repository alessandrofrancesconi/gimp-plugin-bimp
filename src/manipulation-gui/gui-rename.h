#ifndef __GUI_RENAME_H__
#define __GUI_RENAME_H__

#include <gtk/gtk.h>
#include "../bimp-manipulations.h"

#define RENAME_WINDOW_W 340
#define RENAME_WINDOW_H 300

#define ENTRY_W RENAME_WINDOW_W - 80
#define ENTRY_H 30

#define PREVIEW_W RENAME_WINDOW_W - 50
#define PREVIEW_H 40

#define FRAME_HELP_W RENAME_WINDOW_W - 80
#define FRAME_HELP_H RENAME_WINDOW_H - ENTRY_H - PREVIEW_H - 140

GtkWidget* bimp_rename_gui_new(rename_settings, GtkWidget*);
void bimp_rename_save(rename_settings);

#endif
