#ifndef CLIENTSRC_C_WINDOW_H
#define CLIENTSRC_C_WINDOW_H

/**
 * @Usage GTK Windows: Actions, creation and activation
 */

void destroy (GSimpleAction *action, GVariant *parameter, gpointer user_data);

void activate(GtkApplication *client, gpointer user_data);

int createWindow(int argc, char **argv);

#endif //CLIENTSRC_C_WINDOW_H
