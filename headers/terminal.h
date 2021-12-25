#ifndef CLIENTSRC_C_TERMINAL_H
#define CLIENTSRC_C_TERMINAL_H

#include "login.h"

void cmdMain(database *db, fileConfig *config);

int cmdDoubleCheck(database * db, int id);

void cmdTweakConfigs(fileConfig* targetConfig);

void cmdTweakConfLoop(configType* conf, int nbPossibleValues, char possibleValues[][10]);

#endif //CLIENTSRC_C_TERMINAL_H
