#include "../headers/config.h"
#include "configTypes.c"

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