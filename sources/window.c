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

///Used to circumvent a bug with the state-set callback
///Removing debugPointer would lead to a nullPointer exception when state-set is called back
gpointer debugPointer;


/**
 * ##########################################
 * #                                        #
 * #            Callback functions          #
 * #                                        #
 * ##########################################
 */

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

/**
 * @usage treats hasGui button press in the settings window
 * @param configSwitch
 * @param data
 */
void updateHasGuiGeneral(GtkSwitch *configSwitch, gpointer data){
    ///If data has been wiped, correct it
    ///Removing debugPointer would lead to a nullPointer exception when state-set is called back
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
                    gtk_entry_set_text(userInput, "");
                    gtk_entry_set_text(passwordInput, "");
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
 * @usage treat login button press
 * @param registerButton -- register button widget
 * @param data -- user data
 */
void onLogin(GtkButton *registerButton, gpointer data){
    windowData *uData = data;
    GtkLabel * error = GTK_LABEL(gtk_builder_get_object(uData->builder, "loginError"));

    char username[255];
    char password[255];

    GtkEntry * userInput = GTK_ENTRY(gtk_builder_get_object(uData->builder, "loginUsername"));
    GtkEntry * passwordInput = GTK_ENTRY(gtk_builder_get_object(uData->builder, "loginPassword"));

    if(checkForSpeChars(gtk_entry_get_text(userInput)) == CHECK_OK||checkForSpeChars(gtk_entry_get_text(passwordInput))==CHECK_OK){
        if(strlen(gtk_entry_get_text(userInput))<=255&&strlen(gtk_entry_get_text(passwordInput))<=255){
            strcpy(username, gtk_entry_get_text(userInput));
            strcpy(password, gtk_entry_get_text(passwordInput));
            int logStatus = login(uData->db, uData->session,username, password);
            if (logStatus == LOGIN_ERR) gtk_label_set_text(error, "Failed to log in");
            else{
                gtk_entry_set_text(userInput, "");
                gtk_entry_set_text(passwordInput, "");
                gtk_label_set_text(error, " ");
                updateSessionWindow(GTK_WIDGET(gtk_builder_get_object(uData->builder, "sessionWindow")), data);
            }
        }else gtk_label_set_text(error, "Error: Max length is 255 characters");
    }else gtk_label_set_text(error, "Error: Do not use special characters");
}

/**
 * @usage treat scrolling through feeds on the session window
 * @param scroll
 * @param data
 */
void onFeedsScroll(GtkScrollbar * scroll, gpointer data){
    windowData * uData = data;

    GtkLabel * feedsHeader = GTK_LABEL(gtk_builder_get_object(uData->builder, "feeds"));
    GtkAdjustment * scale = GTK_ADJUSTMENT(gtk_builder_get_object(uData->builder, "feedsScale"));
    int feedCount = getFeedCount(uData->db, uData->session->id_user);

    if (feedCount == ACCESS_ERROR) gtk_label_set_text(feedsHeader, "Error: Could not access local database");
    else{

        GtkLabel * feedLabels[] = {
                GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName1")),
                GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName2")),
                GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName3")),
                GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName4")),
                GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName5")),
        };

        GtkWidget * feedStruct[] = {
                GTK_WIDGET(gtk_builder_get_object(uData->builder, "feedOptions1")),
                GTK_WIDGET(gtk_builder_get_object(uData->builder, "feedOptions2")),
                GTK_WIDGET(gtk_builder_get_object(uData->builder, "feedOptions3")),
                GTK_WIDGET(gtk_builder_get_object(uData->builder, "feedOptions4")),
                GTK_WIDGET(gtk_builder_get_object(uData->builder, "feedOptions5")),
        };

        int nbPages = feedCount / 5 + (feedCount % 5 > 0 ? 1 : 0);
        gtk_adjustment_set_upper(scale, nbPages);
        int page = (int) gtk_adjustment_get_value(scale);
        int lim = ((page == nbPages && feedCount % 5 != 0) ? feedCount% 5 : 5);
        char feeds [5][255];
        getFeedsList(uData->db, 5 * (page - 1), lim, uData->session->id_user,feeds);
        for (int i = 0; i < 5; ++i) {
            if(i<lim) {
                gtk_widget_show(feedStruct[i]);
                gtk_label_set_text(feedLabels[i], feeds[i]);
            }else gtk_widget_hide(feedStruct[i]);
        }
    }
}


/**
 * ##########################################
 * #                                        #
 * #        Init&Update functions           #
 * #                                        #
 * ##########################################
 */


/**
 * @usage update session window with user session data when accessed afterlogin
 * @param sessionWindow
 * @param data
 */
void updateSessionWindow(GtkWidget * sessionWindow,gpointer data){
    windowData *uData = data;
    gtk_widget_show(sessionWindow);
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(uData->builder, "loginWindow")));

    GtkLabel * feedsHeader = GTK_LABEL(gtk_builder_get_object(uData->builder, "feeds"));
    GtkAdjustment * scale = GTK_ADJUSTMENT(gtk_builder_get_object(uData->builder, "feedsScale"));
    int feedCount = getFeedCount(uData->db, uData->session->id_user);

    if (feedCount == ACCESS_ERROR) gtk_label_set_text(feedsHeader, "Error: Could not access local database");
    else{

        GtkLabel * feedLabels[] = {
                GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName1")),
                GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName2")),
                GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName3")),
                GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName4")),
                GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName5")),
        };

        GtkWidget * feedStruct[] = {
                GTK_WIDGET(gtk_builder_get_object(uData->builder, "feedOptions1")),
                GTK_WIDGET(gtk_builder_get_object(uData->builder, "feedOptions2")),
                GTK_WIDGET(gtk_builder_get_object(uData->builder, "feedOptions3")),
                GTK_WIDGET(gtk_builder_get_object(uData->builder, "feedOptions4")),
                GTK_WIDGET(gtk_builder_get_object(uData->builder, "feedOptions5")),
        };

        int nbPages = feedCount / 5 + (feedCount % 5 > 0 ? 1 : 0);
        gtk_adjustment_set_upper(scale, nbPages);
        int lim = ((1 == nbPages && feedCount % 5 != 0) ? feedCount% 5 : 5);
        char feeds [5][255];
        getFeedsList(uData->db, 0, lim, uData->session->id_user,feeds);
        for (int i = 0; i < 5; ++i) {
            if(i<lim) {
                gtk_widget_show(feedStruct[i]);
                gtk_label_set_text(feedLabels[i], feeds[i]);
            }else gtk_widget_hide(feedStruct[i]);
        }
        g_signal_connect(scale, "value-changed", G_CALLBACK(onFeedsScroll), data);
    }
}

/**
 * @usage initialises session window callbacks and elements
 * @param window
 * @param data
 */
void initSessionWindow(GtkWidget *window, gpointer data){
    windowData *uData = data;
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), data);
    g_signal_connect(GTK_BUTTON(gtk_builder_get_object(uData->builder,"sessionOptions")),
                     "clicked", G_CALLBACK(updateConfigWindow), data);
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
    g_signal_connect(loginButton, "clicked", G_CALLBACK(onLogin), data);
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

/**
 * @usage update config window when it is called
 * @param button -- the button pressed
 * @param data -- user data
 */
void updateConfigWindow(GtkButton *button, gpointer data){
    windowData *uData = data;

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
    GtkWidget *sessionWindow = GTK_WIDGET(gtk_builder_get_object(builder, "sessionWindow"));

    uData->builder = builder;

    ///Init login window
    initLoginWindow(loginWindow, data);
    initConfigWindow(settingsWindow, data);
    initSessionWindow(sessionWindow, data);

    gtk_builder_connect_signals(builder, data);

    if(uData->session->id_user != LOGIN_ERR){
        gtk_widget_hide(settingsWindow);
        gtk_widget_hide(loginWindow);

        updateSessionWindow(sessionWindow, data);
    }else{
        gtk_widget_hide(sessionWindow);
        gtk_widget_hide(settingsWindow);
        gtk_widget_show(loginWindow);
    }



    gtk_main();
}