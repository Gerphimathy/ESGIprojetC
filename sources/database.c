#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include <string.h>
#include "../headers/macros.h"

#include "../headers/database.h"
#include "../headers/login.h"
#include "../headers/feed.h"


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
 * @usage updates a users' refreshToken
 * @param db -- database
 * @param id -- id of the user
 * @param path -- new refreshToken
 * @return Change_ok if change successful, Change_no if unsuccessful
 */

int updateUserAuth(database * db, int id, char token[255]){
    char *delete = "UPDATE users SET yt_token = @token WHERE _id = @id;";

    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, delete, -1, &db->statement,0);
    sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@id"), id);
    sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@token"), token, strlen(token), NULL);
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

/**
 * @usage count number of lines in table users
 * @param db -- database structure
 * @return ACCESS_ERROR if could not connect to database (for example typo in table name), number of lines otherwise
 */
int getUserCount(database * db){
    char * req = "SELECT count(*) FROM users;";
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement, NULL);

    if (db->databaseConnection != SQLITE_OK) return ACCESS_ERROR;

    db->databaseConnection = sqlite3_step(db->statement);

    if (db->databaseConnection != SQLITE_ROW) {
        sqlite3_finalize(db->statement);
        return ACCESS_ERROR;
    }

    int count = sqlite3_column_int(db->statement, 0);
    sqlite3_finalize(db->statement);
    return count;
}

/**
 * @usage count number of lines in table feed from userId
 * @param db -- database structure
 * @param userId -- id of the feed's owner
 * @return ACCESS_ERROR if could not connect to database (for example typo in table name), number of lines otherwise
 */
int getFeedCount(database * db, int userId){
    char * req = "SELECT count(*) FROM feed WHERE id_user = @id;";
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement, NULL);

    if (db->databaseConnection != SQLITE_OK) return ACCESS_ERROR;

    sqlite3_bind_int(db->statement,1,userId);

    db->databaseConnection = sqlite3_step(db->statement);

    if (db->databaseConnection != SQLITE_ROW) {
        sqlite3_finalize(db->statement);
        return ACCESS_ERROR;
    }

    int count = sqlite3_column_int(db->statement, 0);
    sqlite3_finalize(db->statement);
    return count;
}

/**
 * @usage count number of lines in table rel_ch_feed from feedId
 * @param db -- database structure
 * @param userId -- id of the feed's owner
 * @return ACCESS_ERROR if could not connect to database (for example typo in table name), number of lines otherwise
 */
int getChannelCount(database * db, int feedId){
    char * req = "SELECT count(*) FROM rel_ch_feed WHERE id_feed = @id;";
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement, NULL);

    if (db->databaseConnection != SQLITE_OK) return ACCESS_ERROR;

    sqlite3_bind_int(db->statement,1,feedId);

    db->databaseConnection = sqlite3_step(db->statement);

    if (db->databaseConnection != SQLITE_ROW) {
        sqlite3_finalize(db->statement);
        return ACCESS_ERROR;
    }

    int count = sqlite3_column_int(db->statement, 0);
    sqlite3_finalize(db->statement);
    return count;
}

/**
 * @usage Builds a list of usernames into dest, starting from firstIndex and ending on firstIndex + amount
 * Will check if trying to access out of bounds value
 * @param db -- database structure
 * @param firstIndex -- first index of the user list to get
 * @param amount -- amount of usernames to select
 * @param dest -- Value where the result will be built
 * @return ACCESS_SUCCESS in case of success, ACCESS_FAILURE in case of failure
 */
int getUsernameList(database * db, int firstIndex, int amount, char dest[][255]){
    if(firstIndex+amount > getUserCount(db))return ACCESS_ERROR;

    char * req = "SELECT username FROM users LIMIT @amount OFFSET @firstIndex;";
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement, NULL);
    if(db->databaseConnection != SQLITE_OK){
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db->databaseHandle));
        return ACCESS_ERROR;
    }
    sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@amount"), amount);
    sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@firstIndex"), firstIndex);

    for (int i = 0; i < amount; ++i) {
        int step = sqlite3_step(db->statement);
        if (step != SQLITE_ROW){
            sqlite3_finalize(db->statement);
            return ACCESS_ERROR;
        }
        strcpy(dest[i], sqlite3_column_text(db->statement, 0));
    }
    sqlite3_finalize(db->statement);
    return ACCESS_SUCCESS;
}

/**
 * @usage Builds a list of feeds into dest, starting from firstIndex and ending on firstIndex + amount
 * Will check if trying to access out of bounds value
 * @param db -- database structure
 * @param firstIndex -- first index of the user list to get
 * @param amount -- amount of usernames to select
 * @param dest -- Value where the result will be built
 * @return ACCESS_SUCCESS in case of success, ACCESS_FAILURE in case of failure
 */
int getFeedsList(database * db, int firstIndex, int amount, int userId, char dest[][255]){
    if(firstIndex+amount > getFeedCount(db, userId))return ACCESS_ERROR;

    char * req = "SELECT name FROM feed WHERE id_user = @id LIMIT @amount OFFSET @firstIndex;";
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement, NULL);
    if(db->databaseConnection != SQLITE_OK){
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db->databaseHandle));
        return ACCESS_ERROR;
    }
    sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@id"), userId);
    sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@amount"), amount);
    sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@firstIndex"), firstIndex);

    for (int i = 0; i < amount; ++i) {
        int step = sqlite3_step(db->statement);
        if (step != SQLITE_ROW){
            sqlite3_finalize(db->statement);
            return ACCESS_ERROR;
        }
        strcpy(dest[i], sqlite3_column_text(db->statement, 0));
    }
    sqlite3_finalize(db->statement);
    return ACCESS_SUCCESS;
}