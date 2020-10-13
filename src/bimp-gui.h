#ifndef __BIMP_GUI_H__
#define __BIMP_GUI_H__

#include <gtk/gtk.h>

/* Sequence panel (and children) dimensions */
#define SEQ_BUTTON_W 100
#define SEQ_BUTTON_H 100

/* Option panel (and children) dimensions */
#define FILE_PREVIEW_W 150
#define FILE_PREVIEW_H 130

#define PREVIEW_IMG_W 270
#define PREVIEW_IMG_H 220

void bimp_show_gui(void);
void bimp_refresh_sequence_panel(void);
void bimp_refresh_fileview(void);
void bimp_progress_bar_set(double, char*);
void bimp_show_error_dialog(char*, GtkWidget*);
void bimp_set_busy(gboolean);

extern GtkWidget* bimp_window_main;

#endif
