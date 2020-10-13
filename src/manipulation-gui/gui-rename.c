#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include "gui-rename.h"
#include "../bimp-manipulations.h"
#include "../bimp-gui.h"
#include "../bimp-utils.h"
#include "../plugin-intl.h"

static void check_entrytext (GtkEditable*, gpointer);
static GtkWidget *entry_pattern, *label_preview;

GtkWidget* bimp_rename_gui_new(rename_settings settings, GtkWidget* parent)
{
    GtkWidget *gui, *frame_help;
    GtkWidget *label_help;
    
    gui = gtk_vbox_new(FALSE, 5);
    
    entry_pattern = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_pattern), 50);
    gtk_entry_set_text(GTK_ENTRY(entry_pattern), settings->pattern);
    
    frame_help = gtk_frame_new(_("Keywords"));
    
    label_help = gtk_label_new(g_strconcat(
        RENAME_KEY_ORIG, " = ", _("Original filename (without extension)"), "\n", 
        RENAME_KEY_COUNT, " = ", _("Incremental number"), "\n", 
        RENAME_KEY_DATETIME, " = ", _("Date and time (YYYY-MM-DD_hh-mm)"), NULL));
    gtk_container_add(GTK_CONTAINER(frame_help), label_help);
    
    label_preview = gtk_label_new("");
    
    gtk_box_pack_start(GTK_BOX(gui), entry_pattern, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), frame_help, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gui), label_preview, FALSE, FALSE, 5);
    
    g_signal_connect(G_OBJECT(entry_pattern), "changed", G_CALLBACK(check_entrytext), parent);
    
    return gui;
}

void check_entrytext (GtkEditable *editable, gpointer parent) 
{
    const char* entrytext = gtk_entry_get_text(GTK_ENTRY(entry_pattern));
    if (strstr(entrytext, RENAME_KEY_ORIG) == NULL && strstr(entrytext, RENAME_KEY_COUNT) == NULL) {
        /* check for the presence of mandatory keywords (otherwise it will write on the same filename) */
        gtk_label_set_text(GTK_LABEL(label_preview), g_strdup_printf(_("Can't save!\n'%s' or '%s' symbol must be present."), RENAME_KEY_ORIG, RENAME_KEY_COUNT));
        gtk_dialog_set_response_sensitive (GTK_DIALOG(parent), GTK_RESPONSE_ACCEPT, FALSE);
    } else if (
        /* check for invalid characters (for Windows systems) */
        strstr(entrytext, "\\") != NULL ||
        strstr(entrytext, "/") != NULL ||
        strstr(entrytext, "*") != NULL ||
        strstr(entrytext, ":") != NULL ||
        strstr(entrytext, "?") != NULL ||
        strstr(entrytext, "|") != NULL ||
        strstr(entrytext, ">") != NULL ||
        strstr(entrytext, "<") != NULL) {
        gtk_label_set_text(GTK_LABEL(label_preview), _("Can't save!\nPattern contains invalid characters."));
        gtk_dialog_set_response_sensitive (GTK_DIALOG(parent), GTK_RESPONSE_ACCEPT, FALSE);
    }
    else {
        gtk_label_set_text(GTK_LABEL(label_preview), ""); // TODO: preview text?
        gtk_dialog_set_response_sensitive (GTK_DIALOG(parent), GTK_RESPONSE_ACCEPT, TRUE);
    }    
}

void bimp_rename_save(rename_settings orig_settings) 
{
    const char* newtext = gtk_entry_get_text(GTK_ENTRY(entry_pattern));
    if (strlen(newtext) > 0) {
        orig_settings->pattern = g_strdup(newtext);;
    }
}


