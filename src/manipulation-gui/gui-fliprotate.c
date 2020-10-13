#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <libgimpwidgets/gimpwidgets.h>
#include "gui-fliprotate.h"
#include "../bimp-manipulations.h"
#include "../bimp-manipulations-gui.h"
#include "../bimp-utils.h"
#include "../plugin-intl.h"
    
static GtkWidget *button_flipH, *button_flipV, *combo_rotate;

GtkWidget* bimp_fliprotate_gui_new(fliprotate_settings settings)
{
    GtkWidget *gui, *hbox_flip, *hbox_rotate;
    GtkWidget *label_flip, *label_rotate;
    GtkWidget *align_flip, *align_rotate;
    
    gui = gtk_vbox_new(FALSE, 5);
    
    label_flip = gtk_label_new(g_strconcat(_("Flip"), ":", NULL));
    hbox_flip = gtk_hbox_new(FALSE, 5);
    align_flip = gtk_alignment_new(0.5, 0, 0, 0);
    
    button_flipH = gtk_toggle_button_new_with_label(_("Horizontally"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_flipH), settings->flip_h);
    gtk_button_set_image(GTK_BUTTON(button_flipH), image_new_from_resource("/gimp/plugin/bimp/icons/stock-flip-horizontal.png"));
    gtk_button_set_image_position(GTK_BUTTON(button_flipH), GTK_POS_TOP);
    
    button_flipV = gtk_toggle_button_new_with_label(_("Vertically"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_flipV), settings->flip_v);
    gtk_button_set_image(GTK_BUTTON(button_flipV), image_new_from_resource("/gimp/plugin/bimp/icons/stock-flip-vertical.png"));
    gtk_button_set_image_position(GTK_BUTTON(button_flipV), GTK_POS_TOP);
    
    hbox_rotate = gtk_hbox_new(FALSE, 5);
    align_rotate = gtk_alignment_new(0.5, 0, 0, 0);
    label_rotate = gtk_label_new(g_strconcat(_("Rotation"), ":", NULL));
    combo_rotate = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_rotate), _("None"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_rotate), "90°");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_rotate), "180°");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_rotate), "270°");
    
    int active_index;
    if(!settings->rotate) {
        active_index = 0;
    }
    else {
        switch(settings->rotation_type) {
            case GIMP_ROTATE_90: active_index = 1; break;
            case GIMP_ROTATE_180: active_index = 2; break;
            case GIMP_ROTATE_270: active_index = 3; break;
            default: active_index = 0; break;
        }
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_rotate), active_index);
    
    gtk_box_pack_start(GTK_BOX(hbox_flip), button_flipH, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_flip), button_flipV, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(align_flip), hbox_flip);
    
    gtk_box_pack_start(GTK_BOX(hbox_rotate), label_rotate, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_rotate), combo_rotate, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(align_rotate), hbox_rotate);
    
    gtk_box_pack_start(GTK_BOX(gui), label_flip, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), align_flip, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), align_rotate, FALSE, FALSE, 5);
    
    return gui;
}

void bimp_fliprotate_save(fliprotate_settings orig_settings) 
{
    orig_settings->flip_h = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_flipH));
    orig_settings->flip_v = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_flipV));
    
    int active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_rotate));
    orig_settings->rotate = (active_index > 0);
    if (active_index > 0) {
        if (active_index == 1) {
            orig_settings->rotation_type = GIMP_ROTATE_90;
        }
        else if (active_index == 2) {
            orig_settings->rotation_type = GIMP_ROTATE_180;
        }
        else if (active_index == 3) {
            orig_settings->rotation_type = GIMP_ROTATE_270;
        }
    }
}
