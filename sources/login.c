#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>

#include "../headers/macros.h"
#include "../types/databaseTypes.c"
#include "../headers/database.h"
#include "../headers/login.h"
#include "../headers/config.h"
#include "../types/sessionType.c"


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

/**
 * @usage updates a users' configuration folder
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

/**
 * @usage updates a users' password
 * @param db -- database
 * @param id -- id of the user
 * @param path -- new password
 */
void updateUserPassword(database * db, int id, char password[255]){
    char *delete = "UPDATE users SET password = @pass WHERE _id = @id;";
    unsigned char hash[512];

    hashPass(password, hash);

    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, delete, -1, &db->statement,0);
    sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@id"), id);
    sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@pass"), hash, strlen(hash),NULL);
    sqlite3_step(db->statement);
    sqlite3_finalize(db->statement);
}

/**
 * @usage Will delete a user from the database by, in order, deleting from rel_ch_feed, then feed then finally the user
 * @param db -- database structure
 * @param id -- user id to delete
 */
void deleteUser(database * db, int id){
    char delete[255] = "DELETE FROM rel_ch_feed WHERE id_feed IN (SELECT _id FROM feed WHERE id_user = ?);";

    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, delete, -1, &db->statement,0);
    sqlite3_bind_int(db->statement, 1, id);
    sqlite3_step(db->statement);
    sqlite3_step(db->statement);
    sqlite3_finalize(db->statement);

    strcpy(delete, "DELETE FROM feed WHERE id_user = ?;");
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, delete, -1, &db->statement,0);
    sqlite3_bind_int(db->statement, 1, id);
    sqlite3_step(db->statement);
    sqlite3_finalize(db->statement);

    strcpy(delete, "DELETE FROM users WHERE _id = ?;");
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, delete, -1, &db->statement,0);
    sqlite3_bind_int(db->statement, 1, id);
    sqlite3_step(db->statement);
    sqlite3_finalize(db->statement);
}