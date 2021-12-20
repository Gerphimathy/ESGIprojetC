#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <gtk/gtk.h>

#include "headers/window.h"
#include "headers/config.h"
#include "sources/configTypes.c"

typedef struct commandLineParameters{///List of parameters parsed from command line
    char hasGui[10]; //Uses GUI ? "true/false"

} commandLineParameters;

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

int main(int argc, char **argv) {
    fileConfig masterConfig;
    strcpy(masterConfig.path, "config/main.conf");
    parseConfigFile(&masterConfig);

    commandLineParameters lineParams;

    parseArgs(&lineParams, masterConfig, argc, argv);

    if (strcmp(lineParams.hasGui, "true") == 0) createWindow(0, argv);

    return 0;
}