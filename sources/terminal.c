#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <openssl/sha.h>

#include "../headers/macros.h"
#include "../headers/database.h"
#include "../headers/login.h"
#include "../headers/config.h"
#include "../headers/login.h"
#include "../headers/terminal.h"

/**
 * @usage main loop for CMD execution
 * @param db -- database structure
 * @param config -- master config structure
 */
void cmdMain(database *db, fileConfig* config) {
    char action[255];
    char username[255];
    char pass[255];
    session userSession;

    do {
        fflush(stdin);
        strcpy(action, "none");
        printf("\nChoose Action:"
               "\n\nquit\t\t-\tCloses client"
               "\nlogin\t\t-\tLog into profile"
               "\nregister\t-\tRegister profile"
               "\nconf\t\t-\tEdit master configuration"
               "\n");
        fgets(action, 255, stdin);
        if (action[strlen(action) - 1] == '\n') action[strlen(action) - 1] = '\0';

        if (strcmp(action, "login") == 0) {
            system("cls");
            fflush(stdin);
            printf("Login into profile");

            printf("\nUsername:\t");
            fgets(username, 255, stdin);
            if (username[strlen(username) - 1] == '\n') username[strlen(username) - 1] = '\0';

            fflush(stdin);
            printf("\nPassword:\t");
            fgets(pass, 255, stdin);
            if (pass[strlen(pass) - 1] == '\n') pass[strlen(pass) - 1] = '\0';

            system("cls");
            if (login(db, &userSession, username, pass) == LOGIN_ERR) printf(">>Failed to log in");
            else {
                printf(">>Login Successful");
                cmdSession (db, &userSession, config);
            }
        }
        if (strcmp(action, "register") == 0) {
            system("cls");
            fflush(stdin);
            printf("Register Profile\nDo not use Special characters: '\"\\%%/`");

            printf("\nUsername:\t");
            fgets(username, 255, stdin);
            if (username[strlen(username) - 1] == '\n') username[strlen(username) - 1] = '\0';

            fflush(stdin);
            printf("\nPassword:\t");
            fgets(pass, 255, stdin);
            if (pass[strlen(pass) - 1] == '\n') pass[strlen(pass) - 1] = '\0';

            system("cls");
            int status = registerAccount(db, username, pass);
            if (status == REGISTER_SUCCESS) printf(">>Account Successfully created");
            else fprintf(stderr, ">>Local Account Creation failure");
            if (status == REGISTER_DUPLICATE) fprintf(stderr, "\nProfile with this username already exists");
        }
        if (strcmp(action, "conf") == 0) tweakConfigs(config);

    } while (strcmp(action, "quit") != 0);
}

/**
 * @usage main loop once in a session
 * @param db -- database structure
 * @param userSession -- user session structure
 * @param defaultConfig -- config for the default configuration, copied to userSession if path == none
 */
void cmdSession(database *db, session *userSession, fileConfig *defaultConfig){
    char action[255];
    char subAction[10];
    char password[255];
    
    do {
        fflush(stdin);
        strcpy(action, "none");
        printf("\nChoose Action:"
               "\n\nquit\t\t-\tDisconnects from session"
               "\nfeeds\t\t-\tManage feeds"
               "\nconf\t\t-\tEdit personal configurations"
               "\npass\t\t-\tEdit password"
               "\ndel\t\t-\tDelete profile"
               "\n");
        fgets(action, 255, stdin);
        if (action[strlen(action) - 1] == '\n') action[strlen(action) - 1] = '\0';

        if (strcmp(action, "conf") == 0) {
            if (strcmp(defaultConfig->path,userSession->config.path) == 0) {
                printf("You are currently using the default settings\n"
                       "Would you like to use special configurations for this profile ?\n"
                       "[yes/no]\n\n");
                fgets(subAction, 10, stdin);
                if (subAction[strlen(subAction) - 1] == '\n') subAction[strlen(subAction) - 1] = '\0';
                if(strcmp(subAction, "Yes")==0
                   ||strcmp(subAction, "yes")==0
                   ||strcmp(subAction, "YES")==0) {
                    snprintf(userSession->config.path, 255,"config/user%d.conf", userSession->id_user);
                    parseConfigFile(&userSession->config);
                    updateUserConf(db, userSession->id_user,userSession->config.path);
                    printf("\n>>Configuration File Generated");
                }
            }
            else{
                printf("You are currently using your own settings\n"
                       "Would you like to use the defaults settings on this profile ?\n"
                       "[yes/no]\n\n");
                fgets(subAction, 10, stdin);
                if (subAction[strlen(subAction) - 1] == '\n') subAction[strlen(subAction) - 1] = '\0';
                if(strcmp(subAction, "Yes")==0
                   ||strcmp(subAction, "yes")==0
                   ||strcmp(subAction, "YES")==0) {
                    userSession->config = *defaultConfig;
                    updateUserConf(db, userSession->id_user,"none");
                    printf("\n>>Configuration File Reset");
                }else{
                    tweakConfigs(&userSession->config);
                }
            }
        }

        if (strcmp(action, "pass") == 0){
            if (cmdDoubleCheck(db, userSession->id_user) == CHECK_OK){
                fflush(stdin);
                system("cls");
                printf(">>Credentials verified\n\nChoose the new password\nDo not use special chars: '\"\\%%/`");
                printf("\nPassword:\t");

                fgets(password, 255, stdin);
                if (password[strlen(password) - 1] == '\n') password[strlen(password) - 1] = '\0';

                if (updateUserPassword(db, userSession->id_user, password) == CHANGE_OK) printf("\nUser password Updated");
                else printf("\nFailed to update password");

            }else printf(">>Wrong profile credentials\n");
        }
        if (strcmp(action, "del") == 0){
            if (cmdDoubleCheck(db, userSession->id_user) == CHECK_OK){
                fflush(stdin);
                system("cls");
                printf(">>Credentials verified\n");
                printf("You are about to delete the profile\n"
                       "Are you sure you want to delete the profile ?\n"
                       "[yes/no]\n\n");
                fgets(subAction, 10, stdin);
                if (subAction[strlen(subAction) - 1] == '\n') subAction[strlen(subAction) - 1] = '\0';
                if(strcmp(subAction, "Yes")==0
                   ||strcmp(subAction, "yes")==0
                   ||strcmp(subAction, "YES")==0) {
                    deleteUser(db,userSession->id_user);
                    strcpy(action, "quit");
                }
            }
        }

    } while (strcmp(action, "quit") != 0);
}

/**
 * @usage double checks user credentials before committing to serious change
 * @param db -- database
 * @param id -- userID
 * @return CHECK_OK is the check is successful, CHECK_NO is it is not
 */
int cmdDoubleCheck(database * db, int id){
    char username[255];
    char pass[255];
    printf("\n\nENTER YOUR USERNAME AND PASSWORD TO PROCEED\n\n");

    fflush(stdin);
    printf("Username:\t");
    fgets(username, 255, stdin);
    if (username[strlen(username) - 1] == '\n') username[strlen(username) - 1] = '\0';

    fflush(stdin);
    printf("\nPassword:\t");
    fgets(pass, 255, stdin);
    if (pass[strlen(pass) - 1] == '\n') pass[strlen(pass) - 1] = '\0';

    if (verifyCredentials(db, id, username, pass) == CREDENTIALS_VERIFIED) return CHECK_OK;
    else return CHECK_NO;
}