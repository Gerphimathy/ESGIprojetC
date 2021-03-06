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
#include "../headers/feed.h"

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
        if(nbPages>0){
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
        }else{
            char pageText[255];
            sprintf(pageText, "Page: 1 / 1");
            gtk_label_set_text(pageLabel, pageText);
            for (int i = 0; i < 5; ++i) gtk_label_set_text(profileLabels[i], " ");
        }
    }
}

/**
 * @usage treats activating/deactivating user specific config file
 * @param configSwitch
 * @param data
 */
void changeUserConfig(GtkSwitch *configSwitch, gpointer data){
    ///If data has been wiped, correct it
    ///Removing debugPointer would lead to a nullPointer exception when state-set is called back
    if(data != debugPointer){
        data = debugPointer;
    }
    windowData *uData = data;
    gboolean status = gtk_switch_get_active(configSwitch);

    if(status==TRUE) {
        uData->session->config = *uData->config;
        updateUserConf(uData->db, uData->session->id_user,"none");
    }else {
        snprintf(uData->session->config.path, 255,"config/user%d.conf", uData->session->id_user);
        parseConfigFile(&uData->session->config);
        updateUserConf(uData->db, uData->session->id_user, uData->session->config.path);
    }

    updateConfigWindow(GTK_BUTTON(gtk_builder_get_object(uData->builder, "loginButton")), data);
}

/**
 * @usage treats hasGui button press in the settings window
 * @param configSwitch
 * @param data
 */
void updateHasGui(GtkSwitch *configSwitch, gpointer data){
    ///If data has been wiped, correct it
    ///Removing debugPointer would lead to a nullPointer exception when state-set is called back
    if(data != debugPointer){
        data = debugPointer;
    }
    windowData *uData = data;
    gboolean status = gtk_switch_get_active(configSwitch);
    char *targetVal;
    fileConfig *targetConfig;

    GtkSwitch *generalSwitch = GTK_SWITCH(gtk_builder_get_object(uData->builder, "hasGuiGeneral"));
    GtkSwitch *userSwitch = GTK_SWITCH(gtk_builder_get_object(uData->builder, "hasGuiUser"));

    if(configSwitch == generalSwitch) {
        targetVal = uData->config->hasGui.value;
        targetConfig = uData->config;
    }
    if (configSwitch == userSwitch) {
        targetVal = uData->session->config.hasGui.value;
        targetConfig = &uData->session->config;
    }


    if(status)strcpy(targetVal, "true");
    else strcpy(targetVal, "false");

    buildConfigFile(targetConfig);
}

/**
 * @usage treat register button press
 * @param registerButton -- register button widget
 * @param data -- user data
 */
void onRegister(GtkButton *registerButton, gpointer data){
    windowData *uData = data;
    GtkLabel * error = GTK_LABEL(gtk_builder_get_object(uData->builder, "registerError"));
    GtkAdjustment *scale = GTK_ADJUSTMENT(gtk_builder_get_object(uData->builder, "profilesScale"));

    char username[255];
    char password[255];

    GtkEntry * userInput = GTK_ENTRY(gtk_builder_get_object(uData->builder, "registerUsername"));
    GtkEntry * passwordInput = GTK_ENTRY(gtk_builder_get_object(uData->builder, "registerPassword"));

    if(checkForSpeChars(gtk_entry_get_text(userInput)) == CHECK_OK&&checkForSpeChars(gtk_entry_get_text(passwordInput))==CHECK_OK){
        if(strlen(gtk_entry_get_text(userInput))<=255&&strlen(gtk_entry_get_text(passwordInput))<=255){
            strcpy(username, gtk_entry_get_text(userInput));
            strcpy(password, gtk_entry_get_text(passwordInput));
            int regStatus = registerAccount(uData->db, username, password);
            switch (regStatus) {
                case REGISTER_SUCCESS:
                    gtk_adjustment_set_upper(scale, (int)(getUserCount(uData->db) / 5 + (getUserCount(uData->db) % 5 > 0 ? 1 : 0)));
                    onProfilesListScroll(scale, data);
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
    GtkWidget *settingsWindow = GTK_WIDGET(gtk_builder_get_object(uData->builder, "settingsWindow"));

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
                gtk_widget_hide(settingsWindow);
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

    ///Hide feed renaming/deleting dialogs
    GtkDialog * feedRenameDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedRenameDialog"));
    gtk_widget_hide(GTK_WIDGET(feedRenameDialog));

    GtkLabel * deleteInfoLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "deleteFeedLabel"));
    gtk_label_set_text(deleteInfoLabel, "You are about to delete the following feed:\n\n%s\n\nProceed ?");

    GtkDialog *feedDeleteDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedDeleteDialog"));
    gtk_widget_hide(GTK_WIDGET(feedDeleteDialog));

    ///Get scale and header
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
        if(nbPages>0){
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
        }else for (int i = 0; i < 5; ++i) gtk_widget_hide(feedStruct[i]);
    }
}

/**
 * @usage called when confirming to add a new feed, treats adding the feed and returning errors
 * @param accept -- button pressed
 * @param data - userData
 */
void onFeedAddConfirm(GtkButton * accept, gpointer data){
    windowData * uData = data;

    GtkWidget * window = GTK_WIDGET(gtk_builder_get_object(uData->builder, "sessionWindow"));

    GtkLabel * errorLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "feedAddError"));

    GtkEntry * entry = GTK_ENTRY(gtk_builder_get_object(uData->builder, "feedAddEntry"));

    if(checkForSpeChars(gtk_entry_get_text(entry)) == CHECK_OK){
        if(strlen(gtk_entry_get_text(entry))<=255) {
            int addStatus = createFeed(uData->db, gtk_entry_get_text(entry), uData->session->id_user);
            switch (addStatus) {
                case REGISTER_DUPLICATE:
                    gtk_label_set_text(errorLabel, "Error: A feed by that name already exists");
                    break;
                case REGISTER_ERR:
                    gtk_label_set_text(errorLabel, "Error: Unexpected error while trying to access database");
                    break;
                case REGISTER_SUCCESS:
                    updateSessionWindow(window,data);
                    gtk_label_set_text(errorLabel, "Feed successfully created");
                    break;
                default:
                    break;
            }
        }else gtk_label_set_text(errorLabel, "Error: Name max length is 255 characters");
    }else gtk_label_set_text(errorLabel, "Error: Do not use special characters '\"\\%%/`");
}

/**
 * @usage called when canceling adding a new feed, treats putting everything to nil when quitting
 * @param cancel -- cancel button
 * @param data -- user data
 */
void onFeedAddCancel(GtkButton * cancel, gpointer data){
    windowData * uData = data;
    GtkLabel * errorLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "feedAddError"));
    gtk_label_set_text(errorLabel, " ");

    GtkEntry * entry = GTK_ENTRY(gtk_builder_get_object(uData->builder, "feedAddEntry"));
    gtk_entry_set_text(entry,"");

    GtkDialog *feedAddDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedAddDialog"));
    gtk_widget_hide(GTK_WIDGET(feedAddDialog));
}

/**
 * @usage called when clicking on add new feed, calls the add feed dialog and puts every value to nil
 * @param button -- add feed button
 * @param data -- user data
 */
void callAddFeedDialog(GtkButton * button, gpointer data){
    windowData * uData = data;
    GtkDialog * addFeedDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedAddDialog"));

    GtkLabel * errorLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "feedAddError"));
    gtk_label_set_text(errorLabel, " ");

    GtkEntry * entry = GTK_ENTRY(gtk_builder_get_object(uData->builder, "feedAddEntry"));
    gtk_entry_set_text(entry,"");

    GtkDialog *feedAddDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedAddDialog"));

    gtk_widget_show(GTK_WIDGET(addFeedDialog));
}

/**
 * @usage treats disconnecting from a session by hiding dialog windows and removing data
 * @param logout -- called button
 * @param data -- user data
 */
void onLogout(GtkButton *logout, gpointer data){
    windowData * uData = data;
    uData->session->id_user = LOGIN_ERR;
    uData->selectedFeed = NULL;

    ///Reset info labels
    GtkLabel * deleteInfoLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "deleteFeedLabel"));
    gtk_label_set_text(deleteInfoLabel, "You are about to delete the following feed:\n\n%s\n\nProceed ?");


    ///Hide windows
    GtkWidget *loginWindow = GTK_WIDGET(gtk_builder_get_object(uData->builder, "loginWindow"));
    GtkWidget *settingsWindow = GTK_WIDGET(gtk_builder_get_object(uData->builder, "settingsWindow"));
    GtkWidget *sessionWindow = GTK_WIDGET(gtk_builder_get_object(uData->builder, "sessionWindow"));
    GtkDialog *feedAddDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedAddDialog"));
    GtkDialog *feedRenameDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedRenameDialog"));
    GtkDialog *feedDeleteDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedDeleteDialog"));
    GtkDialog *profileDeleteDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "profileDeleteDialog"));
    GtkDialog *passwordDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "passwordDialog"));

    gtk_widget_hide(settingsWindow);
    gtk_widget_hide(sessionWindow);
    gtk_widget_hide(GTK_WIDGET(feedDeleteDialog));
    gtk_widget_hide(GTK_WIDGET(feedAddDialog));
    gtk_widget_hide(GTK_WIDGET(passwordDialog));
    gtk_widget_hide(GTK_WIDGET(feedRenameDialog));
    gtk_widget_hide(GTK_WIDGET(profileDeleteDialog));

    ///Set Config Switch on general configs
    GtkStack *confStack = GTK_STACK(gtk_builder_get_object(uData->builder, "configStack"));
    GtkWidget *generalConfig = GTK_WIDGET(gtk_builder_get_object(uData->builder, "generalStack"));

    gtk_stack_set_visible_child(confStack, generalConfig);

    ///Re init settings
    // TODO: De activated because of switch bug:
    //  gtk_widget_show(loginWindow);
    gtk_main_quit();
}

/**
 * @usage called when renaming a feed
 * @param confirm -- confirm button
 * @param data -- user data
 */
void onFeedRenameConfirm(GtkButton * confirm, gpointer data){
    windowData * uData = data;

    GtkLabel * errorLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "feedRenameError"));

    GtkEntry * entry = GTK_ENTRY(gtk_builder_get_object(uData->builder, "feedRenameEntry"));

    if(checkForSpeChars(gtk_entry_get_text(entry)) == CHECK_OK){
        if(strlen(gtk_entry_get_text(entry))<=255) {
            int renameStatus = renameFeed(uData->db, gtk_entry_get_text(entry), uData->feedId, uData->session->id_user);
            switch (renameStatus) {
                case REGISTER_DUPLICATE:
                    gtk_label_set_text(errorLabel, "Error: A feed by that name already exists");
                    break;
                case REGISTER_ERR:
                    gtk_label_set_text(errorLabel, "Error: Unexpected error while trying to access database");
                    break;
                case REGISTER_SUCCESS:
                    gtk_label_set_text(errorLabel, "Feed successfully renamed");
                    break;
                default:
                    break;
            }
        }else gtk_label_set_text(errorLabel, "Error: Name max length is 255 characters");
    }else gtk_label_set_text(errorLabel, "Error: Do not use special characters '\"\\%%/`");
}

/**
 * @usage called when canceling renaming a feed, sets everything to 0 and hides the window
 * @param cancel -- pressed button
 * @param data -- user data
 */
void onFeedRenameCancel(GtkButton * cancel, gpointer data){
    windowData * uData = data;

    uData->feedId = -1;

    GtkLabel * errorLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "feedRenameError"));
    gtk_label_set_text(errorLabel, " ");

    GtkEntry * entry = GTK_ENTRY(gtk_builder_get_object(uData->builder, "feedRenameEntry"));
    gtk_entry_set_text(entry,"");

    GtkDialog *feedAddDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedRenameDialog"));
    gtk_widget_hide(GTK_WIDGET(feedAddDialog));
}

/**
 * @usage makes the feed rename dialog window visible and prepares it for usage
 * @param selectedFeed -- the button presse, is compared against the list of button to find the feed
 * @param data -- user data
 */
void callFeedRenameDialog(GtkButton * selectedFeed, gpointer data){
    windowData * uData = data;

    ///Close feed deletion dialog
    GtkLabel * deleteInfoLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "deleteFeedLabel"));
    gtk_label_set_text(deleteInfoLabel, "You are about to delete the following feed:\n\n%s\n\nProceed ?");

    GtkDialog *feedDeleteDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedDeleteDialog"));
    gtk_widget_hide(GTK_WIDGET(feedDeleteDialog));

    ///Get feed buttons and names
    GtkButton * renameFeedButtons[] = {
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedRename1")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedRename2")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedRename3")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedRename4")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedRename5")),
    };

    GtkLabel * feedNames[] = {
            GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName1")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName2")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName3")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName4")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName5")),
    };

    GtkDialog * feedRenameDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedRenameDialog"));

    GtkLabel * errorLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "feedRenameError"));
    gtk_label_set_text(errorLabel, " ");

    GtkEntry * entry = GTK_ENTRY(gtk_builder_get_object(uData->builder, "feedRenameEntry"));
    gtk_entry_set_text(entry,"");

    uData->feedId = -1;
    for(int i = 0; i<5; i++){
        if (renameFeedButtons[i] == selectedFeed)
            uData->feedId = getFeedId(uData->db, gtk_label_get_text(feedNames[i]), uData->session->id_user);
    }

    gtk_widget_show(GTK_WIDGET(feedRenameDialog));
}

/**
 * @usage called when confirming deletion of a feed, deletes the feed and closes the window
 * @param cancel -- pressed button
 * @param data -- user data
 */
void onFeedDeleteConfirm(GtkButton * confirm, gpointer data){
    windowData * uData = data;
    deleteFeed(uData->db, uData->feedId);

    GtkDialog *feedDeleteDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedDeleteDialog"));
    gtk_widget_hide(GTK_WIDGET(feedDeleteDialog));

    uData->feedId = -1;

    GtkWidget *sessionWindow = GTK_WIDGET(gtk_builder_get_object(uData->builder, "sessionWindow"));
    updateSessionWindow(sessionWindow, data);
}

/**
 * @usage called when canceling deletion of a feed, sets selected feed to 0 and hides the window
 * @param cancel -- pressed button
 * @param data -- user data
 */
void onFeedDeleteCancel(GtkButton * cancel, gpointer data){
    windowData * uData = data;

    uData->feedId = -1;

    GtkLabel * infoLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "deleteFeedLabel"));
    gtk_label_set_text(infoLabel, "You are about to delete the following feed:\n\n%s\n\nProceed ?");

    GtkDialog *feedDeleteDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedDeleteDialog"));
    gtk_widget_hide(GTK_WIDGET(feedDeleteDialog));
}

/**
 * @usage makes the feed deletion dialog window visible and prepares it for usage
 * @param selectedFeed -- the button presse, is compared against the list of button to find the feed
 * @param data -- user data
 */
void callFeedDeleteDialog(GtkButton *selectedFeed, gpointer data){
    windowData * uData = data;

    ///Close feed renaming dialog
    GtkDialog *feedRenameDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedRenameDialog"));
    gtk_widget_hide(GTK_WIDGET(feedRenameDialog));

    ///Get feed buttons and names
    GtkButton * deleteFeedButtons[] = {
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedDelete1")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedDelete2")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedDelete3")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedDelete4")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedDelete5")),
    };

    GtkLabel * feedNames[] = {
            GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName1")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName2")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName3")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName4")),
            GTK_LABEL(gtk_builder_get_object(uData->builder, "feedName5")),
    };

    GtkDialog *feedDeleteDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "feedDeleteDialog"));

    GtkLabel * infoLabel = GTK_LABEL(gtk_builder_get_object(uData->builder,"deleteFeedLabel"));

    uData->feedId = -1;
    for(int i = 0; i<5; i++){
        if (deleteFeedButtons[i] == selectedFeed){
            uData->feedId = getFeedId(uData->db, gtk_label_get_text(feedNames[i]), uData->session->id_user);
            char tmpBuffer[1024];
            sprintf(tmpBuffer, "You are about to delete the following feed:\n\n%s\n\nProceed ?", gtk_label_get_text(feedNames[i]));
            gtk_label_set_text(infoLabel, tmpBuffer);
        }
    }

    gtk_widget_show(GTK_WIDGET(feedDeleteDialog));
 }

/**
 * @usage delete profiles and closes session
 * @param confirm -- confirm button
 * @param data -- user data
 */
void onProfileDeleteConfirm(GtkButton *confirm, gpointer data){
    windowData *uData = data;
    GtkDialog *dialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "profileDeleteDialog"));

    deleteUser(uData->db, uData->session->id_user);
    int feedCount = getUserCount(uData->db);
    int nbPages = feedCount / 5 + (feedCount % 5 > 0 ? 1 : 0);

    GtkAdjustment *scale = GTK_ADJUSTMENT(gtk_builder_get_object(uData->builder, "profilesScale"));

    gtk_adjustment_set_upper(scale, nbPages);
    gtk_adjustment_set_value(scale, 1);

    onLogout(confirm, data);
}

/**
 * @usage cancels profile deletion and closes dialog
 * @param confirm -- confirm button
 * @param data -- user data
 */
void onProfileDeleteCancel(GtkButton *cancel, gpointer data){
    windowData *uData = data;
    GtkDialog *dialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "profileDeleteDialog"));
    gtk_widget_hide(GTK_WIDGET(dialog));
}

/**
 * @usage show profile deletion dialog
 * @param button -- delete user button
 * @param data -- user data
 */
void callProfileDeleteDialog(GtkButton * button, gpointer data){
    windowData *uData = data;
    GtkDialog *dialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "profileDeleteDialog"));
    gtk_widget_show(GTK_WIDGET(dialog));
}

/**
 * @usage treats password change
 * @param confirm -- confirm button
 * @param data --user data
 */
void onPasswordConfirm(GtkButton *confirm, gpointer data){
    windowData * uData = data;

    GtkLabel * errorLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "passwordError"));

    GtkEntry * entry = GTK_ENTRY(gtk_builder_get_object(uData->builder, "passwordEntry"));

    if(checkForSpeChars(gtk_entry_get_text(entry)) == CHECK_OK){
        if(strlen(gtk_entry_get_text(entry))<=255) {
            int passStatus = updateUserPassword(uData->db, uData->session->id_user, gtk_entry_get_text(entry));
            switch (passStatus) {
                case CHANGE_NO:
                    gtk_label_set_text(errorLabel, "Error: Unexpected error while trying to access database");
                    break;
                case CHANGE_OK:
                    gtk_label_set_text(errorLabel, "Password successfully changed");
                    break;
                default:
                    break;
            }
        }else gtk_label_set_text(errorLabel, "Error: Max length is 255 characters");
    }else gtk_label_set_text(errorLabel, "Error: Do not use special characters '\"\\%%/`");
}

/**
 * @usage treats closing the password change dialog
 * @param cancel -- cancel button
 * @param data --user data
 */
void onPasswordCancel(GtkButton *cancel, gpointer data){
    windowData * uData = data;

    GtkDialog * passwordDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "passwordDialog"));

    GtkLabel * errorLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "passwordError"));
    gtk_label_set_text(errorLabel, " ");

    GtkEntry * entry = GTK_ENTRY(gtk_builder_get_object(uData->builder, "passwordEntry"));
    gtk_entry_set_text(entry,"");

    gtk_widget_hide(GTK_WIDGET(passwordDialog));
}

/**
 * @usage calls the password change dialog
 * @param button -- change password button
 * @param data -- user data
 */
void callPasswordDialog(GtkButton * button, gpointer data){
    windowData * uData = data;

    GtkDialog * passwordDialog = GTK_DIALOG(gtk_builder_get_object(uData->builder, "passwordDialog"));

    GtkLabel * errorLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "passwordError"));
    gtk_label_set_text(errorLabel, " ");

    GtkEntry * entry = GTK_ENTRY(gtk_builder_get_object(uData->builder, "passwordEntry"));
    gtk_entry_set_text(entry,"");

    gtk_widget_show(GTK_WIDGET(passwordDialog));
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
        if(nbPages>0){
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
        }else for (int i = 0; i < 5; ++i) gtk_widget_hide(feedStruct[i]);
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
    g_signal_connect(GTK_BUTTON(gtk_builder_get_object(uData->builder, "addNewFeed")),
                     "clicked", G_CALLBACK(callAddFeedDialog), data);
    g_signal_connect(GTK_BUTTON(gtk_builder_get_object(uData->builder, "logoutButton")),
                     "clicked", G_CALLBACK(onLogout), data);
    g_signal_connect(GTK_BUTTON(gtk_builder_get_object(uData->builder, "deleteUser")),
                     "clicked", G_CALLBACK(callProfileDeleteDialog), data);
    g_signal_connect(GTK_BUTTON(gtk_builder_get_object(uData->builder, "changePassword")),
                     "clicked", G_CALLBACK(callPasswordDialog), data);

    GtkButton * renameFeeds[] = {
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedRename1")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedRename2")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedRename3")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedRename4")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedRename5")),
    };
    for (int i = 0; i < 5; ++i) g_signal_connect(renameFeeds[i], "clicked", G_CALLBACK(callFeedRenameDialog), data);

    GtkButton * deleteFeeds[] = {
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedDelete1")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedDelete2")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedDelete3")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedDelete4")),
            GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedDelete5")),
    };
    for (int i = 0; i < 5; ++i) g_signal_connect(deleteFeeds[i], "clicked", G_CALLBACK(callFeedDeleteDialog), data);

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
        if(nbPages>0){
            gtk_adjustment_set_upper(scale, nbPages);
            int lim = ((1 == nbPages && userCount % 5 != 0) ? userCount % 5 : 5);
            char profiles [5][255];
            getUsernameList(uData->db, 0, lim, profiles);
            for (int i = 0; i < lim; ++i) gtk_label_set_text(profileLabels[i], profiles[i]);
            char pageText[255];
            sprintf(pageText, "Page: %d / %d", 1, nbPages);
            gtk_label_set_text(pageLabel, pageText);
            g_signal_connect(scale, "value-changed", G_CALLBACK(onProfilesListScroll), data);
        }else{
            gtk_adjustment_set_upper(scale, 1);
            char pageText[255];
            sprintf(pageText, "Page: 1 / 1");
            gtk_label_set_text(pageLabel, pageText);
            for (int i = 0; i < 5; ++i) gtk_label_set_text(profileLabels[i], " ");
        }
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

    ///Page switches
    GtkWidget *configSwitch = GTK_WIDGET(gtk_builder_get_object(uData->builder, "configSwitch"));
    GtkSwitch *defaultSettings = GTK_SWITCH(gtk_builder_get_object(uData->builder, "useDefault"));

    ///Config Switches
    GtkSwitch *hasGuiGeneral = GTK_SWITCH(gtk_builder_get_object(uData->builder, "hasGuiGeneral"));
    GtkSwitch *hasGuiUser = GTK_SWITCH(gtk_builder_get_object(uData->builder, "hasGuiUser"));

    ///Window Signals
    g_signal_connect(settingsWindow, "destroy", G_CALLBACK(gtk_widget_hide_on_delete), data);
    g_signal_connect(settingsWindow, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), data);

    ///Set switch states
    if(strcmp(uData->config->path,uData->session->config.path)==0) gtk_switch_set_active(defaultSettings, TRUE);
    else gtk_switch_set_active(defaultSettings, FALSE);

    if(strcmp(uData->config->hasGui.value,"true")==0) gtk_switch_set_active(hasGuiGeneral, TRUE);
    else gtk_switch_set_active(hasGuiGeneral, FALSE);


    ///Lib Bug: state-set callback frees the data pointer: Has to be countered using global pointer debug

    ///Callbacks
    g_signal_connect(defaultSettings, "state-set", G_CALLBACK(changeUserConfig), data);

    g_signal_connect(hasGuiGeneral, "state-set", G_CALLBACK(updateHasGui), data);
    g_signal_connect(hasGuiUser, "state-set", G_CALLBACK(updateHasGui), data);

    if(uData->session->id_user ==LOGIN_ERR) gtk_widget_hide(configSwitch);
    else{
        gtk_widget_show(configSwitch);
    }
}

/**
 * @usage update config window when it is called
 * @param button -- the button pressed
 * @param data -- user data
 */
void updateConfigWindow(GtkButton *button, gpointer data){
    windowData *uData = data;

    ///Process to set a new value when loading from user config:
    ///1 - Disconnect callback to avoid tripping it up
    ///2 - Set the Value
    ///3 - Reconnect the callback

    GtkWidget *configSwitch = GTK_WIDGET(gtk_builder_get_object(uData->builder, "configSwitch"));

    GtkSwitch * hasGuiUser = GTK_SWITCH(gtk_builder_get_object(uData->builder, "hasGuiUser"));
    GtkSwitch * useDefault = GTK_SWITCH(gtk_builder_get_object(uData->builder, "useDefault"));

    ///Use default settings
    /**1**/ g_signal_handlers_disconnect_by_func(useDefault, G_CALLBACK(changeUserConfig), data);
    /**2**/ if(strcmp(uData->config->path,uData->session->config.path)==0) gtk_switch_set_active(useDefault, TRUE);
            else gtk_switch_set_active(useDefault, FALSE);
    /**3**/ g_signal_connect(useDefault, "state-set", G_CALLBACK(changeUserConfig), data);

    if(uData->session->id_user == LOGIN_ERR) gtk_widget_hide(configSwitch);
    else{
        gtk_widget_show(configSwitch);
        if (strcmp(uData->session->config.path,uData->config->path) == 0){///If they use default configs
            gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(uData->builder, "hasGuiUser")));
            gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(uData->builder, "hasGuiLabelUser")));
        }else{
            ///Has Gui User
            /**1**/ g_signal_handlers_disconnect_by_func(hasGuiUser, G_CALLBACK(updateHasGui), data);
            /**2**/ if(strcmp(uData->session->config.hasGui.value,"true")==0) gtk_switch_set_active(hasGuiUser, TRUE);
                    else gtk_switch_set_active(hasGuiUser, FALSE);
            /**3**/ g_signal_connect(hasGuiUser, "state-set", G_CALLBACK(hasGuiUser), data);

            //TODO: gtk_switch_get_state here ^ will always crash when called a second time
            //TODO: Temp fix is to force quit once disconnecting

            gtk_widget_show(GTK_WIDGET(hasGuiUser));
            gtk_widget_show(GTK_WIDGET(useDefault));
            gtk_widget_show(GTK_WIDGET(gtk_builder_get_object(uData->builder, "hasGuiLabelUser")));
        }
    }
    gtk_widget_show(GTK_WIDGET(gtk_builder_get_object(uData->builder, "settingsWindow")));
}

/**
 * @usage initialises the callbacks on the add feed dialog window
 * @param dialog -- dialog window
 * @param data -- userData
 */
void initFeedAddDialog(GtkDialog *dialog, gpointer data){
    windowData * uData = data;

    GtkLabel * errorLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "feedAddError"));
    gtk_label_set_text(errorLabel, " ");

    GtkEntry * entry = GTK_ENTRY(gtk_builder_get_object(uData->builder, "feedAddEntry"));
    gtk_entry_set_text(entry,"");

    GtkButton * accept = GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedAddConfirm"));
    GtkButton * cancel = GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedAddCancel"));

    g_signal_connect(accept, "clicked", G_CALLBACK(onFeedAddConfirm), data);
    g_signal_connect(cancel, "clicked", G_CALLBACK(onFeedAddCancel), data);

    g_signal_connect(dialog, "destroy", G_CALLBACK(gtk_widget_hide_on_delete), data);
    g_signal_connect(dialog, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), data);
}

/**
 * @usage initializes feed renaming dialog
 * @param dialog -- renaming dialog
 * @param data -- user data
 */
void initFeedRenameDialog(GtkDialog *dialog, gpointer data){
    windowData * uData = data;

    GtkLabel * errorLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "feedRenameError"));
    gtk_label_set_text(errorLabel, " ");

    GtkEntry * entry = GTK_ENTRY(gtk_builder_get_object(uData->builder, "feedRenameEntry"));
    gtk_entry_set_text(entry,"");

    GtkButton * accept = GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedRenameConfirm"));
    GtkButton * cancel = GTK_BUTTON(gtk_builder_get_object(uData->builder, "feedRenameCancel"));

    g_signal_connect(accept, "clicked", G_CALLBACK(onFeedRenameConfirm), data);
    g_signal_connect(cancel, "clicked", G_CALLBACK(onFeedRenameCancel), data);

    g_signal_connect(dialog, "destroy", G_CALLBACK(gtk_widget_hide_on_delete), data);
    g_signal_connect(dialog, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), data);
}

/**
 * @usage initializes feed deletion dialog
 * @param dialog -- deleting dialog
 * @param data -- user data
 */
void initFeedDeleteDialog(GtkDialog *dialog, gpointer data){
    windowData * uData = data;

    GtkLabel * infoLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "deleteFeedLabel"));

    GtkButton * accept = GTK_BUTTON(gtk_builder_get_object(uData->builder, "deleteFeedConfirm"));
    GtkButton * cancel = GTK_BUTTON(gtk_builder_get_object(uData->builder, "deleteFeedCancel"));

    g_signal_connect(accept, "clicked", G_CALLBACK(onFeedDeleteConfirm), data);
    g_signal_connect(cancel, "clicked", G_CALLBACK(onFeedDeleteCancel), data);

    g_signal_connect(dialog, "destroy", G_CALLBACK(gtk_widget_hide_on_delete), data);
    g_signal_connect(dialog, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), data);
}

/**
 * @usage initializes profile deletion dialog
 * @param dialog -- deleting dialog
 * @param data -- user data
 */
void initProfileDeleteDialog(GtkDialog *dialog, gpointer data){
    windowData * uData = data;

    GtkButton * accept = GTK_BUTTON(gtk_builder_get_object(uData->builder, "deleteProfileConfirm"));
    GtkButton * cancel = GTK_BUTTON(gtk_builder_get_object(uData->builder, "deleteProfileCancel"));

    g_signal_connect(accept, "clicked", G_CALLBACK(onProfileDeleteConfirm), data);
    g_signal_connect(cancel, "clicked", G_CALLBACK(onProfileDeleteCancel), data);

    g_signal_connect(dialog, "destroy", G_CALLBACK(gtk_widget_hide_on_delete), data);
    g_signal_connect(dialog, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), data);
}

/**
 * @usage initializes password change dialog callbacks
 * @param dialog -- password change dialog
 * @param data -- user data
 */
void initPasswordDialog(GtkDialog * dialog, gpointer data){
    windowData * uData = data;

    GtkLabel * errorLabel = GTK_LABEL(gtk_builder_get_object(uData->builder, "passwordError"));
    gtk_label_set_text(errorLabel, " ");

    GtkEntry * entry = GTK_ENTRY(gtk_builder_get_object(uData->builder, "passwordEntry"));
    gtk_entry_set_text(entry,"");

    GtkButton * accept = GTK_BUTTON(gtk_builder_get_object(uData->builder, "passwordConfirm"));
    GtkButton * cancel = GTK_BUTTON(gtk_builder_get_object(uData->builder, "passwordCancel"));

    g_signal_connect(accept, "clicked", G_CALLBACK(onPasswordConfirm), data);
    g_signal_connect(cancel, "clicked", G_CALLBACK(onPasswordCancel), data);

    g_signal_connect(dialog, "destroy", G_CALLBACK(gtk_widget_hide_on_delete), data);
    g_signal_connect(dialog, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), data);
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

    ///Get windows from builder
    GtkWidget *loginWindow = GTK_WIDGET(gtk_builder_get_object(builder, "loginWindow"));
    GtkWidget *settingsWindow = GTK_WIDGET(gtk_builder_get_object(builder, "settingsWindow"));
    GtkWidget *sessionWindow = GTK_WIDGET(gtk_builder_get_object(builder, "sessionWindow"));
    GtkDialog *feedAddDialog = GTK_DIALOG(gtk_builder_get_object(builder, "feedAddDialog"));
    GtkDialog *feedRenameDialog = GTK_DIALOG(gtk_builder_get_object(builder, "feedRenameDialog"));
    GtkDialog *feedDeleteDialog = GTK_DIALOG(gtk_builder_get_object(builder, "feedDeleteDialog"));
    GtkDialog *profileDeleteDialog = GTK_DIALOG(gtk_builder_get_object(builder, "profileDeleteDialog"));
    GtkDialog *passwordDialog = GTK_DIALOG(gtk_builder_get_object(builder, "passwordDialog"));

    ///Add builder to data for ease of use
    uData->builder = builder;

    ///Init windows
    initLoginWindow(loginWindow, data);
    initConfigWindow(settingsWindow, data);
    initSessionWindow(sessionWindow, data);
    initFeedAddDialog(feedAddDialog, data);
    initFeedRenameDialog(feedRenameDialog, data);
    initFeedDeleteDialog(feedDeleteDialog, data);
    initProfileDeleteDialog(profileDeleteDialog, data);
    initPasswordDialog(passwordDialog, data);

    ///Connect signals
    gtk_builder_connect_signals(builder, data);

    ///Init from login or session window depending on fast login/register status
    if(uData->session->id_user != LOGIN_ERR){
        gtk_widget_hide(loginWindow);
        updateSessionWindow(sessionWindow, data);
    }else{
        gtk_widget_hide(sessionWindow);
        gtk_widget_show(loginWindow);
    }

    gtk_main();
}