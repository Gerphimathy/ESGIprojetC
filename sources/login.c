#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <openssl/sha.h>

#include "../headers/macros.h"
#include "../headers/database.h"
#include "../headers/login.h"
#include "../headers/config.h"
#include "../headers/feed.h"


/**
* @usage user login and token linking functions
*/

/**
 * @Usage returns user id if successful, LOGIN_ERR(-1) if not
 * @param db -- database structure
 * @param targetSession -- address of the session structure to initialize
 * @param username -- username to attempt login with
 * @param password -- password to attempt login with
 * @return login status (LOGIN_ERR or id)
 */
int login(database *db, session *targetSession, char username[255], char password[255]) {
    unsigned char hash[512];
    char req[1024];
    int id;

    hashPass(password, hash);
    strcpy(req ,"SELECT _id, username, yt_token,  conf_file FROM users WHERE (username = @username AND password = @pass);");
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement,0);

    if (db->databaseConnection == SQLITE_OK){
        sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@username"), username, strlen(username), NULL);
        sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@pass"), hash, strlen(hash), NULL);
        int step = sqlite3_step(db->statement);
        if (step == SQLITE_ROW){
            id = (int) sqlite3_column_int(db->statement, 0);
            targetSession->id_user = id;

            strcpy(targetSession->username, sqlite3_column_text(db->statement, 1));

            strcpy(targetSession->auth.refreshToken, sqlite3_column_text(db->statement, 2));

            strcpy(targetSession->config.path, sqlite3_column_text(db->statement, 3));

            sqlite3_finalize(db->statement);

            if(strcmp(targetSession->auth.refreshToken,"none") != 0){
                refresh_token(&(targetSession->auth));
            }

            if (strcmp(targetSession->config.path,"none") == 0){
                fileConfig defaultConfig;
                strcpy(defaultConfig.path, "config/main.conf");
                parseConfigFile(&defaultConfig);
                targetSession->config = defaultConfig;
            }
            parseConfigFile(&targetSession->config);
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
 * @usage Check if credentials match what's inside the database (id, username and password)
 * @param db -- database structure
 * @param id -- id to verify
 * @param username -- username to verify
 * @param password -- password to verify
 * @return LOGIN_ERR if not verified, CREDENTIALS_VERIFIED if verified
 */
int verifyCredentials(database *db, int id, char username[255], char password[255]){
    unsigned char hash[512];
    char req[1024];

    hashPass(password, hash);

    strcpy(req, "SELECT conf_file FROM users WHERE (_id = @id AND username = @username AND password = @pass);");
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement,NULL);

    if (db->databaseConnection == SQLITE_OK) {
        sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@id"), id);
        sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@username"), username, strlen(username), NULL);
        sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@pass"), hash, strlen(hash), NULL);

        int step = sqlite3_step(db->statement);
        sqlite3_finalize(db->statement);

        if (step == SQLITE_ROW) return CREDENTIALS_VERIFIED;
        else return LOGIN_ERR;
    }else return LOGIN_ERR;
}

/**
 * @usage Hashes and salts password using SHA512
 * @param pass -- password to be treated
 * @param dest -- target that will receive treated password
 */
void hashPass(char *pass, unsigned char dest[512]) {
    SHA512_CTX ctx;
    strcpy(dest, pass);

    strcat(dest, SALT);

    /*TODO:...For some ungodly reason, this does not work
    //TODO: DEBUG
    fprintf(stdout, "\n>>%s\n>>%s\n", pass, dest);

    SHA512_Init(&ctx);
    SHA512_Update(&ctx, dest, strlen(dest));
    SHA512_Final(dest, &ctx);

    //TODO: DEBUG
    fprintf(stdout, "\n>>%s\n", dest);
     */
}

/**
 * @usage Adds account to database after checking if username is not a duplicate
 * @param db -- database structure
 * @param username -- username to be added
 * @param password -- password to be added
 * @return register status -- REGISTER_ERR for execution error, REGISTER_SUCCESS for success, REGISTER_DUPLICATE for already existing account
 */
int registerAccount(database *db, char username [255], char password[255]) {
    unsigned char hash[512];
    char req[1024];
    if (checkForSpeChars(username) == CHECK_NO || checkForSpeChars(password) == CHECK_NO) return REGISTER_ERR;

    hashPass(password, hash);

    strcpy(req, "SELECT _id FROM users WHERE (username = @username);");
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement,NULL);

    if (db->databaseConnection == SQLITE_OK){
        sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@username"), username, strlen(username), NULL);
        int step = sqlite3_step(db->statement);
        sqlite3_finalize(db->statement);

        if (step == SQLITE_ROW) return REGISTER_DUPLICATE;
        else{
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