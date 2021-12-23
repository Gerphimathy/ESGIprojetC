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
               "\nlogin\t\t-\tList Accounts to log into"
               "\nregister\t-\tRegister local account"
               "\nconf\t\t-\tEdit master configuration"
               "\n");
        fgets(action, 255, stdin);
        if (action[strlen(action) - 1] == '\n') action[strlen(action) - 1] = '\0';

        if (strcmp(action, "login") == 0) {
            system("cls");
            fflush(stdin);
            printf("Login into local account");

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
                cmdSession (db, &userSession);
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

            strcpy(targetSession->yt_token, sqlite3_column_text16(db->statement, 2));
            strcpy(targetSession->twt_token, sqlite3_column_text16(db->statement, 3));

            strcpy(targetSession->config.path, sqlite3_column_text16(db->statement, 4));

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
 * @usage Hashes, sanitizes and salts password using SHA512
 * @param pass -- password to be treated
 * @param dest -- target that will receive treated password
 */
void hashPass(char *pass, char dest[512]) {
    SHA512_CTX ctx;
    int len = strlen(pass);
    strcpy(dest, pass);

    char *sanitize = dest;

    while (strpbrk(sanitize,"'\"")!=NULL){
        sanitize = strpbrk(sanitize,"'\"");
        if(*sanitize == '\'') *sanitize = 'q';
        if(*sanitize == '"') *sanitize = 'Q';
    }
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
            strcpy(req, "INSERT INTO users(username, password, yt_token, twt_token, conf_file) VALUES(@username, @pass, '', '', '');");
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

void cmdSession(database *db, session *userSession){

}