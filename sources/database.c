#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include <string.h>

#include "../headers/database.h"


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
                            "link_yt VARCHAR(255) NOT NULL,"
                            "link_twt VARCHAR(255) NOT NULL);"

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

