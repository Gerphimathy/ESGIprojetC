#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <string.h>

//TODO: Would be interesting to add these to a specific .h

void destroy (GSimpleAction *action, GVariant *parameter, gpointer user_data){
    exit(-1);
}

void activate(GtkApplication *client, gpointer user_data) {
    GtkWidget *clientWindow;
    GtkEventController *controller;
    GActionGroup *actions;

    ///Create Window
    clientWindow = gtk_application_window_new(client);
    gtk_window_set_title(GTK_WINDOW (clientWindow), "Client");
    gtk_window_set_default_size(GTK_WINDOW (clientWindow), 800, 800);

    ///Create Actions Manager
    GActionEntry clientActions[] = {
            {"destroy", destroy, NULL, NULL, NULL}
    };
    g_object_add_weak_pointer (G_OBJECT (clientWindow), (gpointer *)&clientWindow);
    actions = (GActionGroup*)g_simple_action_group_new ();
    g_action_map_add_action_entries (G_ACTION_MAP (actions),
                                     clientActions, G_N_ELEMENTS (clientActions),
                                     clientWindow);
    gtk_widget_insert_action_group (clientWindow, "cli", actions);

    ///Create Event Controller
    controller = gtk_shortcut_controller_new();
    gtk_shortcut_controller_set_scope(GTK_SHORTCUT_CONTROLLER (controller), GTK_SHORTCUT_SCOPE_GLOBAL);
    gtk_widget_add_controller(clientWindow, controller);

    ///Controller Shortcuts
    gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER (controller),
                                         gtk_shortcut_new(gtk_keyval_trigger_new(GDK_KEY_F4, GDK_CONTROL_MASK),
                                                          gtk_named_action_new("cli.destroy")));

    ///Activate Window
    gtk_window_present(GTK_WINDOW (clientWindow));
}
int createWindow(int argc, char **argv){
    GtkApplication *client;
    int status;

    //TODO: Choose app id (https://developer.gnome.org/documentation/tutorials/application-id.html)
    client = gtk_application_new("edu.mathAndSAH.clientYT", G_APPLICATION_FLAGS_NONE);

    g_signal_connect (client, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION (client), argc, argv);
    g_object_unref(client);

    return status;
}

typedef struct commandLineParameters{///List of parameters parsed from command line
    char hasGui[10]; //Uses GUI ? "true/false"

} commandLineParameters;

///Putting this in its own .h/.c could be useful

typedef struct configType {
    char name[20]; //name of the config, written to the file to be more readable
    char description[300];//default description of the config, printed out when re-creating config file
    char defaultValue[10]; //default value if it's not present in the config/file needs to be built

    //"%s = %s\n%s\n\n"

    char value[10]; //value of the config, acquired when parsing
}configType;

///Putting this in its own .h/.c could be useful

typedef struct fileConfig {
    FILE *configFile;

    char path[255]; //default path of config file (ex: build/config/masterConf)

    configType hasGui; //list of assigned configs
    //TODO: Make a chained list out of this

}fileConfig;

///Putting this in its own .h/.c could be useful
void assignConfigs(fileConfig* targetConfig){//Loads default values into configuration structure to compare when parsing

    //TODO: Add defaults when adding new config
    strcpy(targetConfig->hasGui.name, "Uses_GUI");
    strcpy(targetConfig->hasGui.description, "#Defines Default GUI mode: 1 for GUI, 0 for command lines");
    strcpy(targetConfig->hasGui.defaultValue, "true");

}

void buildConfigFile(fileConfig *targetConfig){
    targetConfig->configFile = fopen(targetConfig->path, "wb+");

    //TODO: Add fprintfs when adding new config
    fprintf(targetConfig->configFile, "%s = %s\n%s\n\n",
            targetConfig->hasGui.name, targetConfig->hasGui.defaultValue,targetConfig->hasGui.description);

}

///Putting this in its own .h/.c could be useful
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
                else strcpy(config->hasGui.value, config->hasGui.defaultValue);
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