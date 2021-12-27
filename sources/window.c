#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "../headers/window.h"

/**
 * @Usage GTK Windows: Actions, creation and activation
 */


/**
 * @param client -- Main Window for the client
 * @param user_data
 * @usage Activates the main window of the client, links actions and events
 */
void activate(GtkApplication *client, gpointer user_data) {
    GtkBuilder *builder = gtk_builder_new_from_file("layouts/clientMain.glade");

    GtkWidget *loginWindow = GTK_WIDGET(gtk_builder_get_object(builder, "clientWindow"));

    gtk_widget_show(loginWindow);
    ///Create Window
}

/**
 * @param argv
 * @return status
 * @usage Creates the main client window then calls activate
 */
void initWindows(char **argv, gpointer data){
    GtkApplication *client;
    int status;

    //TODO: Choose app id (https://developer.gnome.org/documentation/tutorials/application-id.html)
    client = gtk_application_new("edu.mathAndSAH.clientYT", G_APPLICATION_FLAGS_NONE);

    g_signal_connect (client, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION (client), 0, argv);
    g_object_unref(client);
}