#ifndef CLIENTSRC_C_LOGIN_H
#define CLIENTSRC_C_LOGIN_H

#include "config.h"


/**
* @usage user login and token linking functions
*/

/**
* @usage session structures
*/


/**
 * @usage Created after logging in, linked to feeds
 * avoids having to repeatedly access the database
 */
typedef struct channel{
    int id_twt;
    char name[255];
    char twt_link[255];
    char yt_link[255];
}channel;

/**
 * @usage Created after logging in, feed (contains channels)
 * avoids having to repeatedly access the database
 */
typedef struct feed{
    int id_feed;
    char name[255];
    int feedSize;
    channel* channels;
}feed;

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
    int nbFeeds;
    feed* feeds;
}session;

int registerAccount(database *db, char username [255], char password[255]);

void hashPass(char* pass, unsigned char dest[512]);

void cmdSession(database *db, session *userSession, fileConfig *defaultConfig);

void updateUserConf(database * db, int id, char path[255]);

void updateUserPassword(database * db, int id, char password[255]);

int verifyCredentials(database *db, int id, char username[255], char password[255]);

void deleteUser(database * db, int id);

#endif //CLIENTSRC_C_LOGIN_H