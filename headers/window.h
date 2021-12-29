#ifndef CLIENTSRC_C_WINDOW_H
#define CLIENTSRC_C_WINDOW_H

#include "macros.h"
#include "window.h"
#include "config.h"
#include "database.h"
#include "terminal.h"
#include "feed.h"
#include "login.h"

/**
 * @Usage GTK Windows: Actions, creation and activation
 */

typedef struct windowData{
    database *db;
    fileConfig *config;
    session *session;
    GtkBuilder *builder;
    feed *selectedFeed;
    int feedId;
}windowData;

void initLoginWindow(GtkWidget *loginWindow, gpointer data);

void initWindows(char **argv, gpointer data);

void onProfilesListScroll(GtkAdjustment *scale, gpointer data);

void initConfigWindow(GtkWidget *settingsWindow, gpointer data);

void updateHasGuiGeneral(GtkSwitch *configSwitch, gpointer data);

void updateConfigWindow(GtkButton *button, gpointer data);

void onLogin(GtkButton *registerButton, gpointer data);

void updateSessionWindow(GtkWidget * sessionWindow,gpointer data);

void initSessionWindow(GtkWidget *window, gpointer data);

void onFeedsScroll(GtkScrollbar * scroll, gpointer data);

void onFeedAddConfirm(GtkButton * accept, gpointer data);

void onFeedAddCancel(GtkButton * cancel, gpointer data);

void callAddFeedDialog(GtkButton * button, gpointer data);

void onLogout(GtkButton *logout, gpointer data);

void initFeedRenameDialog(GtkDialog *dialog, gpointer data);

void onFeedRenameConfirm(GtkButton * confirm, gpointer data);

void onFeedRenameCancel(GtkButton * cancel, gpointer data);

void callFeedRenameDialog(GtkButton * selectedFeed, gpointer data);

void initFeedDeleteDialog(GtkDialog *dialog, gpointer data);

void onFeedDeleteCancel(GtkButton * cancel, gpointer data);

void onFeedDeleteConfirm(GtkButton * confirm, gpointer data);

void callFeedDeleteDialog(GtkButton *selectedFeed, gpointer data);

void initProfileDeleteDialog(GtkDialog *dialog, gpointer data);

void onProfileDeleteConfirm(GtkButton *confirm, gpointer data);

void onProfileDeleteCancel(GtkButton *cancel, gpointer data);

void callProfileDeleteDialog(GtkButton * button, gpointer data);

void initPasswordDialog(GtkDialog * dialog, gpointer data);

void onPasswordConfirm(GtkButton *confirm, gpointer data);

void onPasswordCancel(GtkButton *cancel, gpointer data);

void callPasswordDialog(GtkButton * button, gpointer data);

#endif //CLIENTSRC_C_WINDOW_H
