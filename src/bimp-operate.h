#ifndef __BIMP_OPERATE_H__
#define __BIMP_OPERATE_H__

#include <gtk/gtk.h>

void bimp_start_batch(gpointer);
void bimp_stop_operations(void);

typedef struct imageout_str {
	gint32 image_id;
	gint32 drawable_id;
	char* filepath;
	char* filename;
} *image_output;

#endif
