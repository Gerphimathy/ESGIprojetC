#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <gtk/gtk.h>

#include "headers/window.h"

//TODO: Would be interesting to add these to a specific .h

typedef struct commandLineParameters{///List of parameters parsed from command line
    char hasGui[10]; //Uses GUI ? "true/false"

} commandLineParameters;

typedef struct configType {
    char name[20]; //name of the config, written to the file to be more readable
    char description[300];//default description of the config, printed out when re-creating config file

    //"%s = %s\n%s\n\n"

    char value[10]; //value of the config, acquired when parsing, given a default value when initializing
}configType;

typedef struct fileConfig {
    FILE *configFile;

    char path[255]; //default path of config file (ex: build/config/masterConf)

    configType hasGui; //list of assigned configs
    //TODO: Make a chained list out of this

}fileConfig;

void assignConfigs(fileConfig* targetConfig){//Loads default values into configuration structure to compare when parsing

    //TODO: Add defaults when adding new config
    strcpy(targetConfig->hasGui.name, "Uses_GUI");
    strcpy(targetConfig->hasGui.description, "#Defines Default GUI mode: true for GUI, false for command lines");
    strcpy(targetConfig->hasGui.value, "true");

}

void buildConfigFile(fileConfig *targetConfig){
    targetConfig->configFile = fopen(targetConfig->path, "wb+");

    //TODO: Add fprintfs when adding new config
    fprintf(targetConfig->configFile, "%s = %s\n%s\n\n",
            targetConfig->hasGui.name, targetConfig->hasGui.value,targetConfig->hasGui.description);

}

void parseConfigFile(fileConfig* config){//Receives config pointer (needs path already filled), builds the structure
    assignConfigs(config);

    config->configFile = fopen(config->path, "rb+");

    if (config->configFile == NULL) buildConfigFile(config);



    char line[2000];
    fseek(config->configFile, 0, SEEK_SET);
    int i;
    char value[10];
    char name[20];
    while(fgets(line,2000, config->configFile)!= NULL){
        strcpy(value,"");
        strcpy(name,"");
        if(line[0] != '#'){
            sscanf(line, "%s = %s", &name, &value);
            if (strcmp(name, config->hasGui.name)==0){
                if ((strcmp(value,"true")==0)||(strcmp(value,"false")==0))
                    strcpy(config->hasGui.value, value);
            }
        }
        i++;
    }
    fclose(config->configFile);
}

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