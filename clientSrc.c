#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <gtk/gtk.h>
#include <json-c/json.h>
#include <openssl/sha.h>

#include "headers/macros.h"
#include "headers/window.h"
#include "headers/config.h"
#include "sources/configTypes.c"
#include "headers/database.h"
#include "sources/databaseTypes.c"
#include "headers/jsonParse.h"
#include "headers/login.h"


/**
 * @usage receives command line parameters after parsing
 */
typedef struct commandLineParameters{///List of parameters parsed from command line
    char hasGui[10]; //Uses GUI ? "true/false"

} commandLineParameters;

/**
 * @param params -- structure where parameters will be stored once parsed
 * @param config -- config structure
 * @param argc -- number of command line parameters
 * @param argv -- command line parameters
 */
void parseArgs(commandLineParameters* params, fileConfig config, int argc, char **argv){

    strcpy(params->hasGui, config.hasGui.value);

    if (argc > 100){//If over 100 arguments, we consider the syntax to be wrong
        printf("Error: Too many arguments");
        exit(-1);
    }

    for (int i = 1; i < argc; ++i) {//We parse each argument
        if (argv[i][0] == '-'){//If the argument starts with - (parameter)
            switch (argv[i][1]) {//Switch depending on the parameter
                case 'h'://help
                    printf("h\t:\tHelp\nl\t:\tRun without GUI\ng\t:\tRun with GUI\n");
                    exit(0);
                    break;
                case 'l'://non visual, line execution
                    strcpy(params->hasGui, "false");//no gui
                    break;
                case 'g'://visual, gtk execution
                    strcpy(params->hasGui, "true");//has gui
                    break;
                default://Ignore incorrect parameters
                    break;
            }
        }else{
            printf("Arguments formatting error");
            exit(-1);
        }
    }
}

/**
 * Parses config file into config structure
 * Parses command line parameters into parameters structure
 * Creates Window if hasGui is enabled
 * Creates a connection to a database & a database handler
 */
int main(int argc, char **argv) {
    ///Parse Config File
    fileConfig masterConfig;
    strcpy(masterConfig.path, "config/main.conf");
    parseConfigFile(&masterConfig);

    ///Parse line parameters
    commandLineParameters lineParams;
    parseArgs(&lineParams, masterConfig, argc, argv);



    ///Prepare main Database Structure
    database localDatabase;
    strcpy(localDatabase.path, "config/localDatabase.db");
    localDatabase.databaseHandle = prepareDatabase("config/localDatabase.db");
    localDatabase.databaseConnection = sqlite3_open("config/localDatabase.db", &localDatabase.databaseHandle);

    ///Create Window if hasGui parameter is true
    if (strcmp(lineParams.hasGui, "true") == 0) createWindow(0, argv);
    else cmdMain(localDatabase, masterConfig);

    sqlite3_close(localDatabase.databaseHandle);

    return 0;
}