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
                            "password VARCHAR(255) NOT NULL);";

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

