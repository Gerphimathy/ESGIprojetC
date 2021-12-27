#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <signal.h>
#include "../headers/database.h"
#include "../headers/login.h"
#include "../headers/window.h"
#include "../headers/macros.h"

/**
 * @Usage GTK Windows: Actions, creation and activation
 */

void initLoginWindow(GtkWidget *loginWindow, GtkBuilder *builder, gpointer data){
    ///We grab the data
    windowData *uData = data;

    ///We connect the close button to quitting
    g_signal_connect(loginWindow, "destroy", G_CALLBACK(gtk_main_quit), data);
    GtkAdjustment *scale = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "profilesScale"));
    GtkLabel *error = GTK_LABEL(gtk_builder_get_object(builder, "profileListError"));
    GtkLabel *pageLabel = GTK_LABEL(gtk_builder_get_object(builder, "profilesPageLabel"));

    GtkLabel *profileLabels[] = {
            GTK_LABEL(gtk_builder_get_object(builder, "profileName1")),
            GTK_LABEL(gtk_builder_get_object(builder, "profileName2")),
            GTK_LABEL(gtk_builder_get_object(builder, "profileName3")),
            GTK_LABEL(gtk_builder_get_object(builder, "profileName4")),
            GTK_LABEL(gtk_builder_get_object(builder, "profileName5")),
    };


    int userCount = getUserCount(uData->db);
    if (userCount == ACCESS_ERROR) gtk_label_set_text(error, "Error: Could not access local database");
    else{
        int nbPages = userCount / 5 + (userCount % 5 > 0 ? 1 : 0);
        gtk_adjustment_set_upper(scale, nbPages);
        int lim = ((1 == nbPages && userCount % 5 != 0) ? userCount % 5 : 5);
        char profiles [5][255];
        getUsernameList(uData->db, 0, lim, profiles);
        for (int i = 0; i < lim; ++i) gtk_label_set_text(profileLabels[i], profiles[i]);
        char pageText[255];
        sprintf(pageText, "Page: %d / %d", 1, lim);
        gtk_label_set_text(pageLabel, pageText);
    }
}

/**
 * @param argv
 * @return status
 * @usage Creates the main client window then calls activate
 */
void initWindows(char **argv, gpointer data){
    gtk_init(0, &argv);

    GtkBuilder *builder = gtk_builder_new_from_file("layouts/clientMain.glade");
    GtkWidget *loginWindow = GTK_WIDGET(gtk_builder_get_object(builder, "loginWindow"));

    ///Init login window
    initLoginWindow(loginWindow, builder, data);

    gtk_builder_connect_signals(builder, data);

    gtk_widget_show(loginWindow);

    gtk_main();
}