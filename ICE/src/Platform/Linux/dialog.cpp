//
// Created by Thomas Ibanez on 10.12.20.
//

#include <string>
#include <gtk/gtk.h>

const std::string open_native_dialog(std::string const& filter) {

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

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_widget_destroy(dialog);
        return std::string(filename);
    }

    gtk_widget_destroy(dialog);

    return std::string("");
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

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_widget_destroy(dialog);
        return std::string(filename);
    }

    gtk_widget_destroy(dialog);

    return std::string("");
}