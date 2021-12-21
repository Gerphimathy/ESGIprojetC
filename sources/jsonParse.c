#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../headers/jsonParse.h"

/**
 * @usage Functions to parse through json files sent by the API
 */

/**
 * @usage Takes a path and returns NULL if the function could not open the file, or a struct json_object* parsed json of json-c
 * @param path -- path to the json file to parse
 * @return struct json_object* parsedJson -- parsed json structure
 */
struct json_object* parseJson(char path[255]){

    FILE *jsonFile;
    struct json_object *parsedJson;
    unsigned int filesize;

    jsonFile = fopen(path, "r");

    if (jsonFile == NULL){
        fprintf(stderr, "Cannot open file at specified path : %s\n", path);
        return NULL;
    }

    fseek(jsonFile,0, SEEK_END);

    filesize = ftell(jsonFile);

    fseek(jsonFile,0, SEEK_SET);

    char buffer[filesize+(filesize%100)]; //Filesize with a 100 bytes margin
    fread(buffer, filesize, 1, jsonFile);
    fclose(jsonFile);

    parsedJson = json_tokener_parse(buffer);

    return parsedJson;
}