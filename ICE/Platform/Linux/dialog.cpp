//
// Created by Thomas Ibanez on 10.12.20.
//

#include <gtk/gtk.h>

#include <string>

#include "dialog.h"

const std::string open_native_dialog(const std::vector<FileFilter> &filters) {

    GtkWidget *dialog;

    if (!gtk_init_check(NULL, NULL)) {
        return std::string();
    }

    dialog = gtk_file_chooser_dialog_new("Open File",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Open", GTK_RESPONSE_ACCEPT,
                                         NULL);

    std::string result;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename != NULL) {
            result = filename;
            g_free(filename);
        }
    }

    gtk_widget_destroy(dialog);

    return result;
}

const std::string open_native_folder_dialog() {

    GtkWidget *dialog;

    if (!gtk_init_check(NULL, NULL)) {
        return std::string();
    }

    dialog = gtk_file_chooser_dialog_new("Open File",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Open", GTK_RESPONSE_ACCEPT,
                                         NULL);

    std::string result;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename != NULL) {
            result = filename;
            g_free(filename);
        }
    }

    gtk_widget_destroy(dialog);

    return result;
}