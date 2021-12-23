#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>

#include "../headers/macros.h"
#include "databaseTypes.c"
#include "../headers/database.h"
#include "../headers/login.h"
#include "../headers/config.h"
#include "sessionType.c"


/**
* @usage user login and token linking functions
*/

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
            printf("\nRegister local account");

            printf("\nUsername:\t");
            fgets(username, 255, stdin);
            if (username[strlen(username) - 1] == '\n') username[strlen(username) - 1] = '\0';

            printf("\nPassword:\t");
            fgets(pass, 255, stdin);
            if (pass[strlen(pass) - 1] == '\n') pass[strlen(pass) - 1] = '\0';

            system("cls");
            if (registerAccount(db, username, pass) == REGISTER_SUCCESS) printf(">>Account Successfully created");
            else fprintf(stderr, ">>Local Account Creation failure");
        }
        if (strcmp(action, "conf") == 0)tweakConfigs(config);

    } while (strcmp(action, "quit") != 0);
}

/**
 * @Usage returns user id if successful, LOGIN_ERR(-1) if not
 * @param db -- database structure
 * @param targetSession -- address of the session structure to initialize
 * @param username -- username to attempt login with
 * @param password -- password to attempt login with
 * @return login status (LOGIN_ERR or id)
 */
int login(database *db, session *targetSession, char username[255], char password[255]) {
    char hash[512];
    char req[1024];
    int id;

    hashPass(password, hash);
    strcpy(req ,"SELECT _id, username, yt_token, twt_token, conf_file FROM users WHERE (username = @username AND password = @pass);");
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement,0);

    if (db->databaseConnection == SQLITE_OK){
        sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@username"), username, strlen(username), NULL);
        sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@pass"), hash, strlen(hash), NULL);
        int step = sqlite3_step(db->statement);
        if (step == SQLITE_ROW){
            id = (int) sqlite3_column_int(db->statement, 0);
            targetSession->id_user = id;

            strcpy(targetSession->username, sqlite3_column_text16(db->statement, 1));

            strcpy(targetSession->yt_token, sqlite3_column_text(db->statement, 2));
            strcpy(targetSession->twt_token, sqlite3_column_text(db->statement, 3));

            strcpy(targetSession->config.path, sqlite3_column_text(db->statement, 4));

            sqlite3_finalize(db->statement);
            return id;
        } else{
            sqlite3_finalize(db->statement);
            return LOGIN_ERR;
        }
    } else{
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->databaseHandle));
        return LOGIN_ERR;
    }
}

/**
 * @usage Hashes and salts password using SHA512
 * @param pass -- password to be treated
 * @param dest -- target that will receive treated password
 */
void hashPass(char *pass, char dest[512]) {
    SHA512_CTX ctx;
    int len = strlen(pass);
    strcpy(dest, pass);

    strcat(dest, pass+len/2);

    SHA512_Init(&ctx);
    SHA512_Update(&ctx, dest, len);
    SHA512_Final(dest, &ctx);
}

/**
 * @usage Adds account to database after checking if username is not a duplicate
 * @param db -- database structure
 * @param username -- username to be added
 * @param password -- password to be added
 * @return register status -- REGISTER_ERR for execution error, REGISTER_SUCCESS for success, REGISTER_DUPLICATE for already existing account
 */
int registerAccount(database *db, char username [255], char password[255]) {
    char hash[512];
    char req[1024];

    hashPass(password, hash);

    strcpy(req, "SELECT _id FROM users WHERE (username = @username);");
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement,NULL);

    if (db->databaseConnection == SQLITE_OK){
        sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@username"), username, strlen(username), NULL);
        int step = sqlite3_step(db->statement);
        sqlite3_finalize(db->statement);

        if (step == SQLITE_ROW){
            fprintf(stderr, "Account already exists");
            return REGISTER_DUPLICATE;
        } else{
            strcpy(req, "INSERT INTO users(username, password, yt_token, twt_token, conf_file) VALUES(@username, @pass, 'none', 'none', 'none');");
            db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement,NULL);

            if (db->databaseConnection == SQLITE_OK){

                sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@username"), username, strlen(username), NULL);
                sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@pass"), hash, strlen(hash), NULL);

                sqlite3_step(db->statement);
                sqlite3_finalize(db->statement);
                return REGISTER_SUCCESS;

            } else{
                fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->databaseHandle));
                return REGISTER_ERR;
            }
        }
    } else{
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->databaseHandle));
        return REGISTER_ERR;
    }
}

void cmdSession(database *db, session *userSession, fileConfig *defaultConfig){
    char action[255];
    char subAction[10];
    if (strcmp(userSession->config.path,"none") == 0){
        userSession->config = *defaultConfig;
    }
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

    } while (strcmp(action, "quit") != 0);
}

/**
 * @usage updates a users' configuration foler
 * @param db -- database
 * @param id -- id of the user
 * @param path -- path of the config file (none for default)
 */
void updateUserConf(database * db, int id, char path[255]){
    char *delete = "UPDATE users SET conf_file = @path WHERE _id = @id;";
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, delete, -1, &db->statement,0);
    sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@id"), id);
    sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@path"), path, strlen(path),NULL);
    sqlite3_step(db->statement);
    sqlite3_finalize(db->statement);
}