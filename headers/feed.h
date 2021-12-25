#ifndef CLIENTSRC_C_FEED_H
#define CLIENTSRC_C_FEED_H

typedef struct channel{
    int id_twt;
    char name[255];
    char twt_link[255];
    char yt_link[255];
}channel;

typedef struct feed{
    int id_feed;
    char name[255];
    int feedSize;
}feed;

#endif //CLIENTSRC_C_FEED_H