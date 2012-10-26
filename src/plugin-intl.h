#ifndef __PLUGIN_INTL_H__
#define __PLUGIN_INTL_H__

#define GETTEXT_PACKAGE "gimp20-plugin-bimp"

#include <libintl.h>

#define _(String) gettext (String)

#ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#else
#    define N_(String) (String)
#endif


#endif /* __PLUGIN_INTL_H__ */
