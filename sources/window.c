#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <signal.h>
#include "../headers/window.h"

/**
 * @Usage GTK Windows: Actions, creation and activation
 */

/**
 * @param argv
 * @return status
 * @usage Creates the main client window then calls activate
 */
void initWindows(char **argv, gpointer data){
    GtkApplication *client;
    GtkWidget *loginWindow;
    gtk_init(0, argv);

    GtkBuilder *builder = gtk_builder_new_from_file("layouts/clientMain.glade");

    loginWindow = GTK_WIDGET(gtk_builder_get_object(builder, "loginWindow"));
    g_signal_connect(loginWindow, "destroy", G_CALLBACK(gtk_main_quit), data);

    gtk_builder_connect_signals(builder, data);

    gtk_widget_show(loginWindow);

    gtk_main();
}