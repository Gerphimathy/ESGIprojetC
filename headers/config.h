#ifndef CLIENTSRC_C_CONFIG_H
#define CLIENTSRC_C_CONFIG_H

/**
 * @Usage Parsing info from config file
 */

typedef struct fileConfig fileConfig;

typedef struct configType configType;

void tweakConfigs(fileConfig* targetConfig);

void tweakConfLoop(configType* conf, int nbPossibleValues, char possibleValues[][10]);

void assignConfigs(fileConfig* targetConfig);

void buildConfigFile(fileConfig *targetConfig);

void parseConfigFile(fileConfig* config);

#endif //CLIENTSRC_C_CONFIG_H
