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
    GtkBuilder *builder;
}windowData;

void initLoginWindow(GtkWidget *loginWindow, gpointer data);

void initWindows(char **argv, gpointer data);

void onProfilesListScroll(GtkAdjustment *scale, gpointer data);

void initConfigWindow(GtkWidget *settingsWindow, gpointer data);

void updateHasGuiGeneral(GtkSwitch *configSwitch, gpointer data);

void updateConfigWindow(GtkButton *button, gpointer data);

#endif //CLIENTSRC_C_WINDOW_H
