#ifndef __BIMP_SERIALIZE_H__
#define __BIMP_SERIALIZE_H__

#include <gtk/gtk.h>

gboolean bimp_serialize_to_file(gchar*);
gboolean bimp_deserialize_from_file(gchar*);

gboolean parse_curve_file(
    char*, 
    int*, guint8**, 
    int*, guint8**, 
    int*, guint8**, 
    int*, guint8**, 
    int*, guint8**
);

#endif
