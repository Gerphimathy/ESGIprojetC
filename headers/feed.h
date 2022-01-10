#ifndef CLIENTSRC_C_FEED_H
#define CLIENTSRC_C_FEED_H



typedef struct feed{
    int id_feed;
    char name[255];
    int feedSize;
}feed;

int createFeed(database *db, char name [255], int usedId);

int getFeedId(database *db, char name[255], int userId);

int renameFeed(database *db, char newName[255], int feedId,int userId);

void deleteFeed(database *db, int feedId);

#endif //CLIENTSRC_C_FEED_H