#ifndef CLIENTSRC_C_WINDOW_H
#define CLIENTSRC_C_WINDOW_H

#include "macros.h"
#include "window.h"
#include "config.h"
#include "database.h"
#include "terminal.h"

/**
 * @Usage GTK Windows: Actions, creation and activation
 */

typedef struct windowData{
    database * db;
    fileConfig * config;
    session * session;
}windowData;

void initLoginWindow(GtkWidget *loginWindow, GtkBuilder *builder, gpointer data);

void initWindows(char **argv, gpointer data);

#endif //CLIENTSRC_C_WINDOW_H
