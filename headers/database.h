#ifndef CLIENTSRC_C_DATABASE_H
#define CLIENTSRC_C_DATABASE_H

/**
 * @usage Communicating with database
 */

sqlite3* prepareDatabase(char path[255]);
void repairDatabase(sqlite3* database, int connect);

#endif //CLIENTSRC_C_DATABASE_H
