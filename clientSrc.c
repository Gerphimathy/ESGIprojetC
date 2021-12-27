#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <signal.h>
#include <json-c/json.h>
#include <openssl/sha.h>
#include <unistd.h>

#include "headers/macros.h"
#include "headers/window.h"
#include "headers/config.h"
#include "headers/database.h"
#include "headers/terminal.h"

/**
 * @usage receives command line parameters after parsing
 */
typedef struct commandLineParameters{///List of parameters parsed from command line
    char hasGui[10]; //Uses GUI ? "true/false"
    char fastLogin[10]; //Log in ? "true/false"
    char fastRegister[10]; //Register and log in ? "true/false"

    char username[255];
    char password[255];

} commandLineParameters;

/**
 * @param params -- structure where parameters will be stored once parsed
 * @param config -- config structure
 * @param argc -- number of command line parameters
 * @param argv -- command line parameters
 */
void parseArgs(commandLineParameters* params, fileConfig *config, int argc, char **argv){
    int isParam;

    if (argc > 100){//If over 100 arguments, we consider the syntax to be wrong
        printf("Error: Too many arguments");
        exit(-1);
    }
    strcpy(params->hasGui, config->hasGui.value);

    for (int i = 1; i < argc; ++i) {//We parse each argument
        isParam = NOT_PARAM;
        if (argv[i][0] == '-'){//If the argument starts with - (parameter)
            switch (argv[i][1]) {//Switch depending on the parameter
                case 'h'://help
                    printf("h\t\t\t:\tHelp"
                           "\nc\t\t\t:\tRun without GUI"
                           "\ng\t\t\t:\tRun with GUI"
                           "\nl [username][password]\t:\tFast login into session"
                           "\nr [username][password]\t:\tFast register profile and login into session"
                           "(If two parameters are contradictory, only the last parameter is checked)"
                           "\n");
                    exit(0);
                    break;
                case 'c'://non visual, line execution
                    strcpy(params->hasGui, "false");//no gui
                    isParam = IS_PARAM;
                    break;
                case 'g'://visual, gtk execution
                    strcpy(params->hasGui, "true");//has gui
                    isParam = IS_PARAM;
                    break;
                case 'l'://fast login into profile
                    strcpy(params->fastLogin, "true");//fast login
                    strcpy(params->fastRegister, "false");//cancel fast register
                    if (argc >= i+2){
                        strncpy(params->username, argv[++i], 255);
                        strncpy(params->password, argv[++i], 255);
                        isParam = IS_PARAM;
                    }else isParam = NOT_PARAM;
                    break;
                case 'r'://fast register a profile
                    strcpy(params->fastRegister, "true");//fast register
                    strcpy(params->fastLogin, "false");//cancel fast login
                    if (argc >= i+2){
                    strncpy(params->username, argv[++i], 255);
                    strncpy(params->password, argv[++i], 255);
                    isParam = IS_PARAM;
                    }else {
                        printf("Arguments formatting error\nUse -h for help");
                        exit(-1);
                    }
                    break;
                default://Ignore incorrect parameters
                    printf("Unknown parameter : %c\nUse -h for help", argv[i][1]);
                    exit(-1);
                    break;
            }
        }else{
            printf("Arguments formatting error\nUse -h for help");
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

    fileConfig masterConfig;
    commandLineParameters lineParams;
    database localDatabase;
    session fastSession;
    fastSession.id_user = LOGIN_ERR; //By default, we do not have a session open
    int appStatus = APP_STANDBY;

    ///PARAMETER PRIORITY POLICY:
    /**
     * 1 - Line Params
     * 2 - User Config
     * 3 - Default Configs
     */

    ///Parse Default Config File
    strcpy(masterConfig.path, "config/main.conf");
    parseConfigFile(&masterConfig);

    ///Prepare main Database Structure
    strcpy(localDatabase.path, "config/localDatabase.db");
    localDatabase.databaseHandle = prepareDatabase("config/localDatabase.db");
    localDatabase.databaseConnection = sqlite3_open("config/localDatabase.db", &localDatabase.databaseHandle);

    ///Parse line parameters once with default config
    parseArgs(&lineParams, &masterConfig, argc, argv);

    ///Apply fast connection
    if (strcmp(lineParams.fastLogin, "true") == 0){
        int res = login(&localDatabase,&fastSession, lineParams.username, lineParams.password);
        if (res == LOGIN_ERR) {
            printf(">>Failed to log in");
            exit(-1);
        }
        ///Parse again but with fast session config
        parseArgs(&lineParams, &fastSession.config, argc, argv);
    }else if (strcmp(lineParams.fastRegister, "true") == 0){
        int res = registerAccount(&localDatabase, lineParams.username, lineParams.password);

        if (res == REGISTER_ERR){
            printf(">>Error while registering");
            exit(-1);
        } else if (res == REGISTER_DUPLICATE) printf(">>Account Already Exists\n>>Logging in\n");

        res = login(&localDatabase,&fastSession, lineParams.username, lineParams.password);
        if (res == LOGIN_ERR) {
            printf(">>Failed to log in");
            exit(-1);
        }
        ///Parse again but with fast session config
        parseArgs(&lineParams, &fastSession.config, argc, argv);
    }

    ///PARAMETER PRIORITY POLICY:
    /**
     * 1 - Line Params
     * 2 - User Config      ) --> Login and parse command line parameters help to apply this
     * 3 - Default Configs  )
     */

    ///Create Window or Terminal using the priority policy:
    if (strcmp(lineParams.hasGui, "true") == 0) {

        windowData data;
        data.db = &localDatabase;
        data.config = &masterConfig;
        data.session = &fastSession;

        appStatus = APP_LAUNCHED;
        initWindows(argv, &data);
    }
    if (strcmp(lineParams.hasGui, "false") == 0) {
        if (fastSession.id_user != LOGIN_ERR) cmdSession(&localDatabase, &fastSession, &masterConfig);
        else cmdMain(&localDatabase, &masterConfig);
        appStatus = APP_LAUNCHED;
    }

    ///Finish
    sqlite3_close(localDatabase.databaseHandle);
    return 0;
}