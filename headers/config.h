#ifndef CLIENTSRC_C_CONFIG_H
#define CLIENTSRC_C_CONFIG_H

#include <stdio.h>

/**
 * @Usage Parsing info from config file
 */

/**
 * @usage Type structures used to receive config file when parsing information
 */

/**
 * @usage Type of config, is used in fileConfig
 */
typedef struct configType {
    char name[20]; //name of the config, written to the file to be more readable
    char description[300];//default description of the config, printed out when re-creating config file

    //"%s = %s\n%s\n\n"

    char value[10]; //value of the config, acquired when parsing, given a default value when initializing
}configType;

/**
 * @usage List of all configs, as well as
 */
typedef struct fileConfig {
    FILE *configFile;

    char path[255]; //default path of config file (ex: build/config/masterConf)

    configType hasGui; //list of assigned configs
    //TODO: Make a chained list out of this

}fileConfig;

typedef struct configType configType;

void assignConfigs(fileConfig* targetConfig);

void buildConfigFile(fileConfig *targetConfig);

void parseConfigFile(fileConfig* config);

#endif //CLIENTSRC_C_CONFIG_H
