#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>

//TODO: Would be interesting to add these to a specific .h
void destroy (GSimpleAction *action, GVariant *parameter, gpointer user_data){
    exit(0);
}

void activate(GtkApplication *client, gpointer user_data) {
    GtkWidget *clientWindow;
    GtkEventController *controller;
    GActionGroup *actions;

    ///Create Window
    clientWindow = gtk_application_window_new(client);
    gtk_window_set_title(GTK_WINDOW (clientWindow), "Client");
    gtk_window_set_default_size(GTK_WINDOW (clientWindow), 800, 800);

    ///Create Actions Manager
    GActionEntry clientActions[] = {
            {"destroy", destroy, NULL, NULL, NULL}
    };
    g_object_add_weak_pointer (G_OBJECT (clientWindow), (gpointer *)&clientWindow);
    actions = (GActionGroup*)g_simple_action_group_new ();
    g_action_map_add_action_entries (G_ACTION_MAP (actions),
                                     clientActions, G_N_ELEMENTS (clientActions),
                                     clientWindow);
    gtk_widget_insert_action_group (clientWindow, "cli", actions);

    ///Create Event Controller
    controller = gtk_shortcut_controller_new();
    gtk_shortcut_controller_set_scope(GTK_SHORTCUT_CONTROLLER (controller), GTK_SHORTCUT_SCOPE_GLOBAL);
    gtk_widget_add_controller(clientWindow, controller);

    ///Controller Shortcuts
    gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER (controller),
                                         gtk_shortcut_new(gtk_keyval_trigger_new(GDK_KEY_F4, GDK_CONTROL_MASK),
                                                          gtk_named_action_new("cli.destroy")));

    ///Activate Window
    gtk_window_present(GTK_WINDOW (clientWindow));
}

int main(int argc, char **argv) {
    GtkApplication *client;
    int status;

    //TODO: Choose app id (https://developer.gnome.org/documentation/tutorials/application-id.html)
    client = gtk_application_new("edu.mathAndSAH.clientYT", G_APPLICATION_FLAGS_NONE);

    g_signal_connect (client, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION (client), argc, argv);
    g_object_unref(client);

    return status;
}


