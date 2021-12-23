#ifndef CLIENTSRC_C_LOGIN_H
#define CLIENTSRC_C_LOGIN_H

#include "config.h"


/**
* @usage user login and token linking functions
*/

typedef struct channel channel;

typedef struct feed feed;

typedef struct session session;

int registerAccount(database *db, char username [255], char password[255]);

void hashPass(char* pass, char dest[512]);

void cmdSession(database *db, session *userSession, fileConfig *defaultConfig);

void updateUserConf(database * db, int id, char path[255]);

void updateUserPassword(database * db, int id, char password[255]);

int verifyCredentials(database *db, int id, char username[255], char password[255]);



#endif //CLIENTSRC_C_LOGIN_H