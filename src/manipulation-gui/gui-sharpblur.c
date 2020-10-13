#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include "gui-sharpblur.h"
#include "../bimp-manipulations.h"
#include "../plugin-intl.h"

static GtkWidget *scale_sharpblur;

GtkWidget* bimp_sharpblur_gui_new(sharpblur_settings settings)
{
    GtkWidget *gui, *hbox_control;
    GtkWidget *label_sharp, *label_blur;
    
    gui = gtk_vbox_new(FALSE, 5);
    
    hbox_control = gtk_hbox_new(FALSE, 5);
    label_sharp = gtk_label_new(_("More sharpen"));
    gtk_misc_set_alignment(GTK_MISC(label_sharp), 0.5, 0.7);
    scale_sharpblur = gtk_hscale_new_with_range(-100, 100, 1);
    gtk_range_set_value(GTK_RANGE(scale_sharpblur), settings->amount);
    gtk_widget_set_size_request (scale_sharpblur, SCALE_AMOUNT_W, -1);
    label_blur = gtk_label_new(_("More blurred"));
    gtk_misc_set_alignment(GTK_MISC(label_blur), 0.5, 0.7);
    
    gtk_box_pack_start(GTK_BOX(hbox_control), label_sharp, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_control), scale_sharpblur, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_control), label_blur, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(gui), hbox_control, FALSE, FALSE, 0);
    
    return gui;
}

void bimp_sharpblur_save(sharpblur_settings orig_settings) 
{
    orig_settings->amount = (int)gtk_range_get_value(GTK_RANGE(scale_sharpblur));
}

