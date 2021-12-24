#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include <string.h>
#include "../headers/macros.h"

#include "../headers/database.h"
#include "../headers/login.h"


/**
 * @usage Communicating with database
 */

/**
 * @usage prepares database file, calls repairDatabase to check the integrity of the database in case of tempering/deletion
 * @param path -- path to the databasefile
 * @return database -- database handle
 */
sqlite3* prepareDatabase(char path[255]){
    sqlite3 *database;
    FILE * testPresence;
    int connect;

    testPresence = fopen(path, "r");

    if(testPresence == NULL){
        fclose(testPresence);
        testPresence = fopen(path, "w");
        if(testPresence == NULL) {
            fprintf(stderr, "Cannot create database:\n");
            exit(-1);
        }
    }

    fclose(testPresence);
    connect = sqlite3_open(path, &database);
    repairDatabase(database, connect);

    return database;
}

/**
 * @usage Checks the integrity of the database and repairs it in case of tempering/deletion of the file
 * @param database -- database handle
 * @param connect -- database connection
 */
void repairDatabase(sqlite3* database, int connect){
    char *errMsg = 0;

    char *creationRequest = "CREATE TABLE IF NOT EXISTS users("
                            "_id INTEGER PRIMARY KEY,"
                            "username VARCHAR(255) NOT NULL,"
                            "password CHAR(512) NOT NULL,"
                            "yt_token VARCHAR(255),"
                            "twt_token VARCHAR(255),"
                            "conf_file VARCHAR(255));"

                            "CREATE TABLE IF NOT EXISTS feed("
                            "_id INTEGER PRIMARY KEY,"
                            "name VARCHAR(255) NOT NULL,"
                            "id_user INTEGER NOT NULL,"
                            "FOREIGN KEY(id_user) REFERENCES users(_id));"

                            "CREATE TABLE IF NOT EXISTS channel("
                            "_id INTEGER PRIMARY KEY,"
                            "yt_link VARCHAR(255) NOT NULL,"
                            "twt_link VARCHAR(255) NOT NULL);"

                            "CREATE TABLE IF NOT EXISTS rel_ch_feed("
                            "id_feed INTEGER NOT NULL,"
                            "id_channel INTEGER NOT NULL,"
                            "FOREIGN KEY(id_feed) REFERENCES feed(_id)"
                            "FOREIGN KEY(id_channel) REFERENCES channel(_id)"
                            "PRIMARY KEY (id_channel, id_feed));"
                            ;

    if (connect != SQLITE_OK){
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(database));
        sqlite3_close(database);
        exit(-1);
    }
    connect = sqlite3_exec(database, creationRequest, 0,0, &errMsg);

    if (connect != SQLITE_OK){
        fprintf(stderr, "Cannot write into database: %s\n", sqlite3_errmsg(database));
        sqlite3_free(errMsg);
        sqlite3_close(database);
        exit(-1);
    }
}

/**
 * @usage Check for special characters in parameters before sending to the database
 * @param string -- string to verify
 * @return CHECK_OK if no forbidden special chars, CHECK_NO for forbidden special chars
 */
int checkForSpeChars(char *string){
    if (strpbrk(string, "'\"\\%%/`") == NULL) return CHECK_OK;
    else return CHECK_NO;
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
 * @return Change_ok if change successful, Change_no if unsuccessful
 */
int updateUserPassword(database * db, int id, char password[255]){
    char *delete = "UPDATE users SET password = @pass WHERE _id = @id;";
    unsigned char hash[512];

    if(checkForSpeChars(password)==CHECK_NO) return CHANGE_NO;

    hashPass(password, hash);

    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, delete, -1, &db->statement,0);
    sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@id"), id);
    sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@pass"), hash, strlen(hash),NULL);
    sqlite3_step(db->statement);
    sqlite3_finalize(db->statement);
    return CHANGE_OK;
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