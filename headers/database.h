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

int checkForSpeChars(char *string);

void updateUserConf(database * db, int id, char path[255]);

int updateUserPassword(database * db, int id, char password[255]);

void deleteUser(database * db, int id);

///We tried having these 3 functions in a single one
///However SQLITE3 will not accept dynamic table name

int getUserCount(database * db);

int getFeedCount(database * db, int userId);

int getChannelCount(database * db, int feedId);

#endif //CLIENTSRC_C_DATABASE_H
