#ifndef CLIENTSRC_C_YTAPI_H
#define CLIENTSRC_C_YTAPI_H

#include <stdint.h>
#include <curl/curl.h>

typedef struct channel{
    char name[255];
    char id[50];
}channel;

struct video{
    char title[255];
    time_t timestamp;
    char* embed;
    char id[40];
};

struct auth{
    char authKey[255];
    char refreshToken[255];
};

struct channelList{
    size_t size;
    struct channel *array;
};

struct videoList{
    size_t size;
    struct video *array;
};

struct string{
    size_t size;
    char* string;
};

void free_string(struct string *string);

void free_video_list(struct videoList *list);

void free_channel_list(struct channelList *list);

time_t isotime_to_timestamp(char * isotime);

char* timestamp_to_isotime(time_t timestamp);

void rand_str(char *dest, size_t length);

size_t write_callback(char *data, size_t size, size_t nmemb, void *answer);

struct string send_req(char* path, char **headers, uint32_t numArg, CURLoption protocol);

struct channelList get_subscribtions(char* authKey);

struct videoList get_videos_from_channel_list(char* authKey, struct channelList channelList, char* publishedAfter, char* publishedBefore);

int get_video_info_from_id(char* authKey, struct video *video);

void refresh_token(struct auth* auth);

struct auth get_token();

int open_embed(struct video video);

#endif //CLIENTSRC_C_YTAPI_H
