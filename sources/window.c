#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <signal.h>

#include "../headers/database.h"
#include "../headers/login.h"
#include "../headers/window.h"
#include "../headers/macros.h"
#include "../headers/config.h"

/**
 * @Usage GTK Windows: Actions, creation and activation
 */

gpointer debugPointer;

/**
 * @usage Called when scrolling down the profiles list to update the list
 * @param scale -- scrollbar scale
 * @param data -- userData
 */
void onProfilesListScroll(GtkAdjustment *scale, gpointer data){
    windowData *uData = data;

    GtkLabel *error = GTK_LABEL(gtk_builder_get_object(uData->builder, "profileListError"));

    GtkLabel *profileLabels[] = {
            GTK_LABEL(gtk_builder_get_object(uData->builder, "profileName1")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "profileName2")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "profileName3")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "profileName4")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "profileName5")),
    };

    GtkLabel *pageLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "profilesPageLabel"));

    int userCount = getUserCount(uData->db);

    if (userCount == ACCESS_ERROR) gtk_label_set_text(error, "Error: Could not access local database");
    else{
        int nbPages = userCount / 5 + (userCount % 5 > 0 ? 1 : 0);
        int page = (int) gtk_adjustment_get_value(scale);
        int lim = ((page == nbPages && userCount % 5 != 0) ? userCount % 5 : 5);
        char profiles [5][255];
        getUsernameList(uData->db, 5 * (page - 1), lim, profiles);
        for (int i = 0; i < 5; ++i) {
            if(i<lim) gtk_label_set_text(profileLabels[i], profiles[i]);
            else gtk_label_set_text(profileLabels[i], " ");
        }
        char pageText[255];
        sprintf(pageText, "Page: %d / %d", page, nbPages);
        gtk_label_set_text(pageLabel, pageText);
    }
}

void updateHasGuiGeneral(GtkSwitch *configSwitch, gpointer data){
    if(data != debugPointer){
        data = debugPointer;
    }
    windowData *uData = data;
    gboolean status = gtk_switch_get_active(configSwitch);
    if(status)strcpy(uData->config->hasGui.value , "true");
    else strcpy(uData->config->hasGui.value , "false");

    buildConfigFile(uData->config);
}

/**
 * @usage treat register button press
 * @param registerButton -- register button widget
 * @param data -- user data
 */
void onRegister(GtkButton *registerButton, gpointer data){
    windowData *uData = data;
    GtkLabel * error = GTK_LABEL(gtk_builder_get_object(uData->builder, "registerError"));

    char username[255];
    char password[255];

    GtkEntry * userInput = GTK_ENTRY(gtk_builder_get_object(uData->builder, "registerUsername"));
    GtkEntry * passwordInput = GTK_ENTRY(gtk_builder_get_object(uData->builder, "registerPassword"));

    if(checkForSpeChars(gtk_entry_get_text(userInput)) == CHECK_OK||checkForSpeChars(gtk_entry_get_text(passwordInput))==CHECK_OK){
        if(strlen(gtk_entry_get_text(userInput))<=255&&strlen(gtk_entry_get_text(passwordInput))<=255){
            strcpy(username, gtk_entry_get_text(userInput));
            strcpy(password, gtk_entry_get_text(passwordInput));
            int regStatus = registerAccount(uData->db, username, password);
            switch (regStatus) {
                case REGISTER_SUCCESS:
                    gtk_label_set_text(error, "Profile successfully created");
                    break;
                case REGISTER_ERR:
                    gtk_label_set_text(error, "Error encountered while trying to access database");
                    break;
                case REGISTER_DUPLICATE:
                    gtk_label_set_text(error, "Error: A profile by that name already exists");
                    break;
                default:
                    break;
            }
        }else gtk_label_set_text(error, "Error: Max length is 255 characters");
    }else gtk_label_set_text(error, "Error: Do not use special characters");
}

/**
 * Initialise login window
 * @param loginWindow -- pointer to loginWindow
 * @param builder -- pointer to glade file parser
 * @param data -- pointer to user data structure
 */
void initLoginWindow(GtkWidget *loginWindow, gpointer data){
    ///We grab the data
    windowData *uData = data;

    ///We connect the close button to quitting
    g_signal_connect(loginWindow, "destroy", G_CALLBACK(gtk_main_quit), data);
    GtkAdjustment *scale = GTK_ADJUSTMENT(gtk_builder_get_object(uData->builder, "profilesScale"));
    GtkLabel *listError = GTK_LABEL(gtk_builder_get_object(uData->builder, "profileListError"));
    GtkLabel *pageLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "profilesPageLabel"));

    GtkButton *registerButton = GTK_BUTTON(gtk_builder_get_object(uData->builder, "registerButton"));
    GtkButton *loginButton = GTK_BUTTON(gtk_builder_get_object(uData->builder, "loginButton"));
    GtkButton *loginSettings = GTK_BUTTON(gtk_builder_get_object(uData->builder, "loginSettings"));

    GtkLabel *profileLabels[] = {
            GTK_LABEL(gtk_builder_get_object(uData->builder, "profileName1")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "profileName2")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "profileName3")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "profileName4")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "profileName5")),
    };

    int userCount = getUserCount(uData->db);
    if (userCount == ACCESS_ERROR) gtk_label_set_text(listError, "Error: Could not access local database");
    else{
        int nbPages = userCount / 5 + (userCount % 5 > 0 ? 1 : 0);
        gtk_adjustment_set_upper(scale, nbPages);
        int lim = ((1 == nbPages && userCount % 5 != 0) ? userCount % 5 : 5);
        char profiles [5][255];
        getUsernameList(uData->db, 0, lim, profiles);
        for (int i = 0; i < lim; ++i) gtk_label_set_text(profileLabels[i], profiles[i]);
        char pageText[255];
        sprintf(pageText, "Page: %d / %d", 1, nbPages);
        gtk_label_set_text(pageLabel, pageText);
        g_signal_connect(scale, "value-changed", G_CALLBACK(onProfilesListScroll), data);
    }

    g_signal_connect(registerButton, "clicked", G_CALLBACK(onRegister), data);
    g_signal_connect(loginSettings, "clicked", G_CALLBACK(updateConfigWindow), data);
}

/**
 * @usage initializes config window
 * @param options -- pressed button, not important
 * @param data -- user data
 */
void initConfigWindow(GtkWidget *settingsWindow, gpointer data){
    windowData *uData = data;

    GtkWidget *configSwitch = GTK_WIDGET(gtk_builder_get_object(uData->builder, "configSwitch"));

    GtkSwitch *hasGuiGeneral = GTK_SWITCH(gtk_builder_get_object(uData->builder, "hasGuiGeneral"));

    g_signal_connect(settingsWindow, "destroy", G_CALLBACK(gtk_widget_hide_on_delete), data);
    g_signal_connect(settingsWindow, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), data);

    if(strcmp(uData->config->hasGui.value,"true")==0) gtk_switch_set_active(hasGuiGeneral, TRUE);
    else gtk_switch_set_active(hasGuiGeneral, FALSE);

    ///Lib Bug: Frees the data pointer: Has to be countered using global pointer debug
    g_signal_connect(hasGuiGeneral, "state-set", G_CALLBACK(updateHasGuiGeneral), data);

    if(uData->session->id_user ==LOGIN_ERR) gtk_widget_hide(configSwitch);
    else{
        gtk_widget_show(configSwitch);
    }
    updateConfigWindow(GTK_BUTTON(gtk_builder_get_object(uData->builder,"loginSettings")), data);
}

void updateConfigWindow(GtkButton *button, gpointer data){
    windowData *uData = data;
    if (debugPointer != data){
        debugPointer = data;
    }

    GtkWidget *configSwitch = GTK_WIDGET(gtk_builder_get_object(uData->builder, "configSwitch"));

    if(uData->session->id_user ==LOGIN_ERR) gtk_widget_hide(configSwitch);
    else{
        gtk_widget_show(configSwitch);
    }

    gtk_widget_show(GTK_WIDGET(gtk_builder_get_object(uData->builder, "settingsWindow")));
}

/**
 * @param argv
 * @return status
 * @usage Creates the main client window then calls activate
 */
void initWindows(char **argv, gpointer data){
    gtk_init(0, &argv);
    windowData *uData = data;
    debugPointer = data;

    GtkBuilder *builder = gtk_builder_new_from_file("layouts/clientMain.glade");
    GtkWidget *loginWindow = GTK_WIDGET(gtk_builder_get_object(builder, "loginWindow"));
    GtkWidget *settingsWindow = GTK_WIDGET(gtk_builder_get_object(builder, "settingsWindow"));

    uData->builder = builder;

    ///Init login window
    initLoginWindow(loginWindow, data);
    initConfigWindow(settingsWindow, data);

    gtk_builder_connect_signals(builder, data);

    gtk_widget_show(loginWindow);
    gtk_widget_hide(settingsWindow);

    gtk_main();
}