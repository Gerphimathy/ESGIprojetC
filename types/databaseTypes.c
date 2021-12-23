#include <stdio.h>
#include <sqlite3.h>

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