#ifndef CLIENTSRC_C_DATABASE_H
#define CLIENTSRC_C_DATABASE_H

/**
 * @usage Communicating with database
 */

/**
 * @usage database information and usage structures
 */

/**
 * @usage Main Database structure, contains handle, connection, path and error message
 */
typedef struct database{
    sqlite3* databaseHandle;
    int databaseConnection;
    char path[255];
    char *errmsg;
    sqlite3_stmt *statement;
}database;

sqlite3* prepareDatabase(char path[255]);
void repairDatabase(sqlite3* database, int connect);

#endif //CLIENTSRC_C_DATABASE_H
