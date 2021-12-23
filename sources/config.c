#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../headers/config.h"
#include "configTypes.c"

/**
 * @Usage Parsing info from config file
 */

/**
 * @usage Tweak config file in cmd mode
 * @param targetConfig -- config file to tweak
 */
void tweakConfigs(fileConfig* targetConfig){

    parseConfigFile(targetConfig);
    char vals[2][10];

    //TODO: Add to this when adding new configs
    strcpy(vals[0],"true");
    strcpy(vals[1],"false");
    tweakConfLoop(&targetConfig->hasGui, 2, vals);

    buildConfigFile(targetConfig);

}


/**
 * @usage Loop to tweak specific config in cmd mode, used by tweakConfigs
 * @param conf -- target configuration to tweak
 * @param nbPossibleValues -- Number of possible values
 * @param possibleValues -- List of possible values
 */
void tweakConfLoop(configType* conf, int nbPossibleValues, char possibleValues[][10]){
    char assignedVal[10];
    short err = 0;
    do {
        system("cls");
        if(err){
            fprintf(stderr, ">>Please enter one of the indicated possible values\n");
        }
        err = 1;
        fprintf(stdout, "%s: %s\n", conf->name, conf->description);
        for (int i = 0; i < nbPossibleValues; ++i) {
            if(strcmp(conf->value, possibleValues[i])==0) fprintf(stdout, "|>>%s|", possibleValues[i]);
            else fprintf(stdout, "|%s|", possibleValues[i]);
        }
        printf("\n");
        fflush(stdin);
        fgets(assignedVal,10, stdin);
        if (assignedVal[strlen(assignedVal) - 1] == '\n') assignedVal[strlen(assignedVal) - 1] = '\0';
        for (int i = 0; i < nbPossibleValues && err; ++i) {
            if(strcmp(assignedVal, possibleValues[i])==0){
                err = 0;
                strcpy(conf->value, assignedVal);
            }
        }
    }while(err);
}

/**
 * @usage called by parseConfigFile to build config with default values
 * @param targetConfig -- initialized config structure
 */
void assignConfigs(fileConfig* targetConfig){//Loads default values into configuration structure to compare when parsing

    //TODO: Add defaults when adding new config
    strcpy(targetConfig->hasGui.name, "Uses_GUI");
    strcpy(targetConfig->hasGui.description, "#Defines Default GUI mode: true for GUI, false for command lines");
    strcpy(targetConfig->hasGui.value, "true");

}

/**
 * @usage called by parseConfigFile when config file is missing to build a default config file
 * @param targetConfig -- config structure containing path to config file
 */
void buildConfigFile(fileConfig *targetConfig){
    targetConfig->configFile = fopen(targetConfig->path, "wb+");

    //TODO: Add fprintfs when adding new config
    fprintf(targetConfig->configFile, "%s = %s\n%s\n\n",
            targetConfig->hasGui.name, targetConfig->hasGui.value,targetConfig->hasGui.description);

    fclose(targetConfig->configFile);
}

/**
 * @usage parses config file into config structure, path needs to be initialized before calling
 * @param config -- target config structure where that will recieve result of config file parse
 */
void parseConfigFile(fileConfig* config){//Receives config pointer (needs path already filled), builds the structure
    assignConfigs(config);

    config->configFile = fopen(config->path, "rb+");

    if (config->configFile == NULL) buildConfigFile(config);

    char line[2000];
    fseek(config->configFile, 0, SEEK_SET);
    int i = 0;
    char value[10];
    char name[20];
    while(fgets(line,2000, config->configFile)!= NULL){
        strcpy(value,"");
        strcpy(name,"");
        if(line[0] != '#'){
            sscanf(line, "%s = %s", &name, &value);
            //TODO: Add to this when adding new configs
            if (strcmp(name, config->hasGui.name)==0){
                if ((strcmp(value,"true")==0)||(strcmp(value,"false")==0))
                    strcpy(config->hasGui.value, value);
            }
        }
        i++;
    }
    fclose(config->configFile);
}