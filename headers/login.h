#ifndef CLIENTSRC_C_LOGIN_H
#define CLIENTSRC_C_LOGIN_H

#include "config.h"


/**
* @usage user login and token linking functions
*/

/**
 * @usage Created after logging in, stores info of logged in user
 * avoids having to repeatedly access the database
 */
typedef struct session{
    int id_user;
    char username[255];
    char yt_token[255];
    char twt_token[255];
    fileConfig config;
}session;

int login(database *db, session *targetSession, char username[255], char password[255]);

int registerAccount(database *db, char username [255], char password[255]);

void hashPass(char* pass, unsigned char dest[512]);

void cmdSession(database *db, session *userSession, fileConfig *defaultConfig);

int verifyCredentials(database *db, int id, char username[255], char password[255]);



#endif //CLIENTSRC_C_LOGIN_H