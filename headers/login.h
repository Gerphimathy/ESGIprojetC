#ifndef CLIENTSRC_C_LOGIN_H
#define CLIENTSRC_C_LOGIN_H

#include "config.h"


/**
* @usage user login and token linking functions
*/

typedef struct channel channel;

typedef struct feed feed;

typedef struct session session;

void cmdMain(database db, fileConfig config);

int login(database db, session *targetSession, char username[255], char password[255]);

int registerAccount(database db, char username [255], char password[255]);

void hashPass(char* pass, char dest[512]);

#endif //CLIENTSRC_C_LOGIN_H
