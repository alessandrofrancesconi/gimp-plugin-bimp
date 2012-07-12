#ifndef __BIMP_OPERATE_H__
#define __BIMP_OPERATE_H__

#include <gtk/gtk.h>

gboolean bimp_start_batch(GtkWidget*);
void bimp_stop_operations(void);

typedef struct imageout_str {
	gint32 image_id;
	gint32 drawable_id;
	char* filepath;
	char* filename;
} *image_output;

#endif
