#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "../headers/ytapi.h"

void free_string(struct string *string){
    free(string->string);
    string->size = 0;
}

void free_video_list(struct videoList *list){
    for(int i = 0; i < list->size; ++i){
        free((list->array + i * (sizeof(struct video)))->embed);
        (list->array + i * (sizeof(struct video)))->embed = NULL;
    }
    free(list->array);
    list->array=NULL;
    list->size = 0;
}

void free_channel_list(struct channelList *list){
    free(list->array);
    list->array = NULL;
    list->size = 0;
}

time_t isotime_to_timestamp(char * isotime){

    time_t timestamp;
    struct tm time_struct;
    sscanf(isotime, "%d-%d-%dT%d:%d:%dZ", &(time_struct.tm_year), &(time_struct.tm_mon), &(time_struct.tm_mday), &(time_struct.tm_hour), &(time_struct.tm_min), &(time_struct.tm_sec));
    time_struct.tm_year -= 1900;
    timestamp = mktime(&time_struct);
    return timestamp;
}

char* timestamp_to_isotime(time_t *timestamp){
    struct tm *time_struct;
    char* isotime;
    isotime = malloc(sizeof(char) * 21);
    if(isotime == NULL)return NULL;

    time_struct = gmtime(timestamp);

    sprintf(isotime,"%04d-%02d-%02dT%02d:%02d:%02dZ",time_struct->tm_year+1900, time_struct->tm_mon+1, time_struct->tm_mday,
            time_struct->tm_hour, time_struct->tm_min, time_struct->tm_sec);
    isotime[20] = '\0';
    return isotime;
}

size_t write_callback(char *data, size_t size, size_t nmemb, void *answer){
    size_t realsize = size * nmemb;
    struct string *mem = (struct string *)answer;

    printf("Ici\n");

    char *ptr = realloc(mem->string, mem->size + realsize + 1);
    if(ptr == NULL)
        return 0;  /* out of memory! */

    mem->string = ptr;
    memcpy(&(mem->string[mem->size]), data, realsize);
    mem->size += realsize;
    mem->string[mem->size] = 0;

    return realsize;

}

struct string send_req(char* path, char **headers, uint32_t numArg, CURLoption protocol){

    struct string answer = {0, NULL};

    CURL *curl = curl_easy_init();  //handle
    if(!curl) return answer;

    CURLcode res;
    struct curl_slist *slist = NULL;


    if(numArg) {
        slist = curl_slist_append(slist, *headers);
        int i;
        for(i = 1; i < numArg; i++){
            slist = curl_slist_append(slist, *(headers+i));
        }


    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);


    curl_easy_setopt(curl, CURLOPT_URL, path);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&answer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, protocol, 1L); // use CURLOPT_HTTPGET or CURLOPT_POST
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);


    res = curl_easy_perform(curl);


    printf("%d\n", res);

    if(slist != NULL)curl_slist_free_all(slist);
    curl_easy_cleanup(curl);
    return answer;
}

struct channelList get_subscribtions(char* authKey){


    char *address = malloc(110*sizeof(char ));
    char** headers;
    char base_address[255] = "https://www.googleapis.com/youtube/v3/subscriptions?mine=true&maxResults=10&part=snippet";
    struct string answer;
    struct json_object *parsed_json, *pageInfo, *nextPageToken, *totalResults, *items, *item, *snippet, *title, *resourceId, *channelId;
    struct channelList list;

    strcpy(address, base_address);

    headers = malloc(1 * sizeof(char *));
    char authorization[255] = "Authorization:Bearer  ";
    strcat(authorization, authKey);
    *headers = authorization;

    int i = 0;

    do{

        answer = send_req(address, headers, 1, CURLOPT_HTTPGET);

        parsed_json = json_tokener_parse(answer.string);

        json_object_object_get_ex(parsed_json, "pageInfo", &pageInfo);
        json_object_object_get_ex(parsed_json, "nextPageToken", &nextPageToken);
        json_object_object_get_ex(pageInfo, "totalResults", &totalResults);
        list.size = json_object_get_uint64(totalResults);
        list.array = malloc(list.size * sizeof(struct channel));


        json_object_object_get_ex(parsed_json, "items", &items);

        for(int j = 0; j < json_object_array_length(items); ++j){
            item = json_object_array_get_idx(items, j);
            json_object_object_get_ex(item, "snippet", &snippet);
            json_object_object_get_ex(snippet, "title", &title);
            strncpy((list.array + 10 * i + j)->name,255,json_object_get_string(title));
            json_object_object_get_ex(snippet, "resourceId", &resourceId);
            json_object_object_get_ex(resourceId, "channelId", &channelId);
            strncpy((list.array + 10 * i + j)->id,255,json_object_get_string(channelId));

        }


        free_string(&answer);
        strcpy(address, base_address);
        strcat(address, "&pageToken=");
        strcat(address, json_object_get_string(nextPageToken));


        ++i;
    }while(i < list.size / 10 + (list.size%10 ? 1 : 0 ));

    free(headers);

    return list;

}

int get_video_info_from_id(char* authKey, struct video *video){

    char* address = malloc( ( 65 + strlen( video->id ) ) * sizeof( char ));
    char** headers;
    struct string answer;
    struct json_object *parsed_json, *items, *item, *player, *embed;

    strcpy(address, "https://youtube.googleapis.com/youtube/v3/videos?part=player&id=");
    strcat(address, video->id);

    headers = malloc(1 * sizeof(char *));
    char authorization[255] = "Authorization:Bearer  ";
    strcat(authorization, authKey);
    *headers = authorization;

    answer = send_req(address, headers, 1, CURLOPT_HTTPGET);

    parsed_json = json_tokener_parse(answer.string);

    json_object_object_get_ex(parsed_json, "items", &items);
    item = json_object_array_get_idx(items, 0);
    json_object_object_get_ex(item, "player", &player);
    json_object_object_get_ex(player, "embedHtml", &embed);
    video->embed = malloc(json_object_get_string_len(embed)+1);
    if(video->embed == NULL) return EXIT_FAILURE;

    strcpy(video->embed,json_object_get_string(embed));

    free(address);
    free(answer.string);

}