#ifndef CLIENTSRC_C_TERMINAL_H
#define CLIENTSRC_C_TERMINAL_H

#include "login.h"

void cmdMain(database *db, fileConfig *config);

int login(database *db, session *targetSession, char username[255], char password[255]);

int cmdDoubleCheck(database * db, int id);

#endif //CLIENTSRC_C_TERMINAL_H
