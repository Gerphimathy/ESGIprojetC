#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <winsock2.h>
#include <windows.h>
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

char* timestamp_to_isotime(time_t timestamp){
    struct tm *time_struct;
    char* isotime;
    isotime = malloc(sizeof(char) * 21);
    if(isotime == NULL)return NULL;

    time_struct = gmtime(&timestamp);

    sprintf(isotime,"%04d-%02d-%02dT%02d:%02d:%02dZ",time_struct->tm_year+1900, time_struct->tm_mon+1, time_struct->tm_mday,
            time_struct->tm_hour, time_struct->tm_min, time_struct->tm_sec);
    isotime[20] = '\0';
    return isotime;
}

void rand_str(char *dest, size_t length) {
    srand(time(NULL));
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                     "";

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}


size_t write_callback(char *data, size_t size, size_t nmemb, void *answer){
    size_t realsize = size * nmemb;
    struct string *mem = (struct string *)answer;


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

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

    }



    curl_easy_setopt(curl, CURLOPT_URL, path);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&answer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, protocol, 1L); // use CURLOPT_HTTPGET or CURLOPT_POST

    if(protocol == CURLOPT_POST){
        char* data = path;
        int i = 0;
        while(data[i++] != '?');
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    }


    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);


    res = curl_easy_perform(curl);



    if(slist != NULL)curl_slist_free_all(slist);
    curl_easy_cleanup(curl);
    return answer;
}

struct channelList get_subscriptions(char* authKey){

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
        int check = json_object_object_get_ex(parsed_json, "nextPageToken", &nextPageToken);
        json_object_object_get_ex(pageInfo, "totalResults", &totalResults);
        if(i == 0) {
            list.size = json_object_get_uint64(totalResults);
            list.array = malloc((list.size+1) * sizeof(struct channel));
        }

        json_object_object_get_ex(parsed_json, "items", &items);

        for(int j = 0; j < json_object_array_length(items); ++j){
            item = json_object_array_get_idx(items, j);
            json_object_object_get_ex(item, "snippet", &snippet);
            json_object_object_get_ex(snippet, "title", &title);
            strncpy((list.array + 10 * i + j)->name,json_object_get_string(title),255);
            json_object_object_get_ex(snippet, "resourceId", &resourceId);
            json_object_object_get_ex(resourceId, "channelId", &channelId);
            strncpy((list.array + 10 * i + j)->id,json_object_get_string(channelId),255);


        }



        free_string(&answer);
        strcpy(address, base_address);
        strcat(address, "&pageToken=");
        if(check)
            strcat(address, json_object_get_string(nextPageToken));


        ++i;
    }while(i < list.size / 10 + (list.size%10 ? 1 : 0 ));

    free(headers);
    return list;

}

struct videoList get_videos_from_channel_list(char* authKey, struct channelList channelList, char* publishedAfter, char* publishedBefore){ //publishedBefore and after use an ISO8601 date time (YYYY-MM-DDThh:mm:ssZ)

    struct videoList videoList = {0, NULL};
    char** headers;
    struct string answer;
    char address[255], tmp_address[255];
    char base_address[255] = "https://youtube.googleapis.com/youtube/v3/search?part=snippet&maxResults=10&publishedAfter=";
    struct json_object *parsed_json, *pageInfo, *nextPageToken, *totalResults, *items, *item, *snippet, *title, *id, *videoId, *publishedAt;

    strcat(base_address, publishedAfter);
    strcat(base_address, "&publishedBefore=");
    strcat(base_address, publishedBefore);
    strcat(base_address, "&channelId=");

    headers = malloc(1 * sizeof(char *));
    char authorization[255] = "Authorization:Bearer  ";
    strcat(authorization, authKey);
    *headers = authorization;

    for (int i = 0; i < channelList.size; ++i) {
        strcpy(tmp_address, base_address);
        strcat(tmp_address, channelList.array[i].id);

        int j = 0;
        int total_result;
        size_t last_size = videoList.size;

        strcpy(address, tmp_address);

        do{

            answer = send_req(address, headers, 1, CURLOPT_HTTPGET);

            parsed_json = json_tokener_parse(answer.string);

            json_object_object_get_ex(parsed_json, "pageInfo", &pageInfo);
            int check = json_object_object_get_ex(parsed_json, "nextPageToken", &nextPageToken);
            json_object_object_get_ex(pageInfo, "totalResults", &totalResults);
            total_result = json_object_get_uint64(totalResults);
            if(j == 0) {
                videoList.size += total_result;
                videoList.array = realloc(videoList.array, (videoList.size+1) * sizeof(struct video));
            }

            json_object_object_get_ex(parsed_json, "items", &items);


            for(int k = 0; k < json_object_array_length(items); ++k){
                item = json_object_array_get_idx(items, k);
                json_object_object_get_ex(item, "snippet", &snippet);
                json_object_object_get_ex(snippet, "publishedAt", &publishedAt);
                (videoList.array + 10 * j + k + last_size)->timestamp = isotime_to_timestamp((char*)json_object_get_string(publishedAt));
                json_object_object_get_ex(snippet, "title", &title);
                strncpy((videoList.array + 10 * j + k + last_size)->title,json_object_get_string(title),255);
                json_object_object_get_ex(item, "id", &id);
                json_object_object_get_ex(id, "videoId", &videoId);
                strncpy((videoList.array + 10 * j + k + last_size)->id,  json_object_get_string(videoId), 255);


            }


            free_string(&answer);
            strcpy(address, tmp_address);
            strcat(address, "&pageToken=");
            if(check)
                strcat(address, json_object_get_string(nextPageToken));


            ++j;
        }while(j < total_result / 10 + (total_result%10 ? 1 : 0 ));


    }

    free(headers);

    return videoList;

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
    free_string(&answer);

}


void refresh_token(struct auth* auth){
    char address[512] = "https://oauth2.googleapis.com/token?client_id=564525896258-9cmrbhbp5qc1gqis13baauv3a3qqvnae.apps.googleusercontent.com&client_secret=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&grant_type=refresh_token&refresh_token=";
    strcat(address, auth->refreshToken);
    struct string answer;
    struct json_object *parsed_json, *access_token;

    answer = send_req(address, NULL, 0, CURLOPT_POST);

    parsed_json = json_tokener_parse(answer.string);
    json_object_object_get_ex(parsed_json, "access_token", &access_token);
    strcpy(auth->authKey, json_object_get_string(access_token) );

    free_string(&answer);

}

struct auth get_token(){

    struct string answer;
    struct auth auth;
    int i, j;
    char reply[2001];
    char verif[128];
    char code[200];
    char address_1[1000] = "https://accounts.google.com/o/oauth2/v2/auth?client_id=564525896258-9cmrbhbp5qc1gqis13baauv3a3qqvnae.apps.googleusercontent.com&response_type=code&scope=https://www.googleapis.com/auth/youtube&redirect_uri=http://localhost:9999&code_challenge=";
    char address_2[1000] = "https://oauth2.googleapis.com/token?client_id=564525896258-9cmrbhbp5qc1gqis13baauv3a3qqvnae.apps.googleusercontent.com&client_secret=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&grant_type=authorization_code&redirect_uri=http://localhost:9999&code=";
    struct json_object *parsed_json, *access_token, *refresh_token;

    rand_str(verif, rand()%85 + 43);
    strcat(address_1, verif);

    printf("1\n");

    ShellExecute(NULL, "open", address_1, NULL,NULL, SW_SHOWNORMAL);

    WSADATA WSAData;
    WSAStartup(MAKEWORD(2,0), &WSAData);

    printf("2\n");

    SOCKET sock;
    SOCKADDR_IN sin, csin;
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    sin.sin_family = AF_INET;
    sin.sin_port = htons(9999);
    sock = socket(AF_INET,SOCK_STREAM,0);
    bind(sock, (SOCKADDR *)&sin, sizeof(sin));


    listen(sock, 0);
    int s = 0;
    int sizeof_csin = sizeof(csin);
    printf("3\n");
    s = accept(sock, (SOCKADDR *) &csin, &sizeof_csin);
    printf("4\n");
    if (s != INVALID_SOCKET) {
        printf("5\n");
        recv(s , reply , 2000 , 0);
        send(s, "Wesh", 5, 0);

    }
    closesocket(s);
    closesocket(sock);

    printf("6\n");
    WSACleanup();
    printf("7\n");

    i = 0;
    j = 0;



    while(reply[i++] != '=');
    while(reply[i + ++j] != '&');
    strncpy(code, reply+i, j);
    code[j] = '\0';

    printf("8\n");

    strcat(address_2, code);
    strcat(address_2, "&code_verifier=");
    strcat(address_2, verif);



    answer = send_req(address_2, NULL, 0, CURLOPT_POST);



    parsed_json = json_tokener_parse(answer.string);

    json_object_object_get_ex(parsed_json, "access_token", &access_token);
    strcpy(auth.authKey, json_object_get_string(access_token) );
    json_object_object_get_ex(parsed_json, "refresh_token", &refresh_token);
    strcpy(auth.refreshToken, json_object_get_string(refresh_token));


    free_string(&answer);


    return auth;


}

int open_embed(struct video video){
    if(video.embed == NULL)
        return 1;
    FILE *fp = fopen("temp_vid.html","w+");
    fputs(video.embed, fp);
    fclose(fp);
    ShellExecute(NULL, "open", "temp_vid.html", NULL,NULL, SW_SHOWNORMAL);
    return 0;
}