#ifndef __BIMP_OPERATE_H__
#define __BIMP_OPERATE_H__

#include <gtk/gtk.h>

typedef struct imageout_str {
    gint32 image_id;
    gint* drawable_ids;
    gint drawable_count;
    char* filepath;
    char* filename;
} *image_output;

void bimp_start_batch(gpointer);
void bimp_init_batch(void);
void bimp_apply_drawable_manipulations(image_output, gchar*, gchar*); 

#endif
