#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <openssl/sha.h>

#include "../headers/macros.h"
#include "../headers/database.h"
#include "../headers/login.h"
#include "../headers/config.h"
#include "../headers/feed.h"

/**
 * @usage Adds feed to database after checking if feed name is not a duplicate for this user's
 * @param db -- database structure
 * @param name -- username of the new feed
 * @param password -- password to be added
 * @return register status -- REGISTER_ERR for execution error, REGISTER_SUCCESS for success, REGISTER_DUPLICATE for already existing feed
 */
int createFeed(database *db, char name [255], int userId) {
    char req[1024];
    if (checkForSpeChars(name) == CHECK_NO) return REGISTER_ERR;

    strcpy(req, "SELECT _id FROM feed WHERE (id_user = @userId AND name = @name);");
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement,NULL);

    if (db->databaseConnection == SQLITE_OK){
        sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@name"), name, strlen(name), NULL);
        sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@userId"), userId);
        int step = sqlite3_step(db->statement);
        sqlite3_finalize(db->statement);

        if (step == SQLITE_ROW) return REGISTER_DUPLICATE;
        else{
            strcpy(req, "INSERT INTO feed(name, id_user) VALUES(@name ,@userId);");
            db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement,NULL);

            if (db->databaseConnection == SQLITE_OK){

                sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@name"), name, strlen(name), NULL);
                sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@userId"), userId);

                sqlite3_step(db->statement);
                sqlite3_finalize(db->statement);
                return REGISTER_SUCCESS;

            } else{
                fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->databaseHandle));
                return REGISTER_ERR;
            }
        }
    } else{
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->databaseHandle));
        return REGISTER_ERR;
    }
}

/**
 * @usage Get id of a feed from its name and userId
 * @param db -- database structure
 * @param name -- name of the feed
 * @param userId -- id of the user
 * @return ACCESS_ERROR (-1) if the feed does not exist, the feed's id if it exists
 */
int getFeedId(database *db, char name[255], int userId){
    char req[1024];

    strcpy(req, "SELECT _id FROM feed WHERE (id_user = @userId AND name = @name);");
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement,NULL);

    if (db->databaseConnection == SQLITE_OK) {
        sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@name"), name, strlen(name), NULL);
        sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@userId"), userId);
        int step = sqlite3_step(db->statement);
        if (step != SQLITE_ROW) {
            sqlite3_finalize(db->statement);
            return ACCESS_ERROR;
        }
        else {
            int ret = sqlite3_column_int(db->statement,0);
            sqlite3_finalize(db->statement);
            return ret;
        }
    }else return ACCESS_ERROR;
}

/**
 * @usage Renames the given feed
 * @param db -- database structure
 * @param newName -- new name to be given to the feed
 * @param feedId -- id of the feed to be rename
 * @param userId -- id of the user of the feed, used to check for duplicates
 * @return REGISTER_SUCCESS if renamed, REGISTER_ERR if error trying to access or REGISTER_DUPLICATE if name is already taken
 */
int renameFeed(database *db, char newName[255], int feedId, int userId){

    char req[1024];
    if (checkForSpeChars(newName) == CHECK_NO) return REGISTER_ERR;

    strcpy(req, "SELECT _id FROM feed WHERE (id_user = @userId AND name = @newName);");
    db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement,NULL);

    if (db->databaseConnection == SQLITE_OK){
        sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@newName"), newName, strlen(newName), NULL);
        sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@userId"), userId);
        int step = sqlite3_step(db->statement);
        sqlite3_finalize(db->statement);

        if (step == SQLITE_ROW) return REGISTER_DUPLICATE;
        else{
            strcpy(req, "UPDATE feed SET name = @newName WHERE _id = @feedId;");
            db->databaseConnection = sqlite3_prepare_v2(db->databaseHandle, req, -1, &db->statement,NULL);

            if (db->databaseConnection == SQLITE_OK){
                sqlite3_bind_text(db->statement, sqlite3_bind_parameter_index(db->statement, "@newName"), newName, strlen(newName), NULL);
                sqlite3_bind_int(db->statement, sqlite3_bind_parameter_index(db->statement, "@feedId"), feedId);

                sqlite3_step(db->statement);
                sqlite3_finalize(db->statement);
                return REGISTER_SUCCESS;

            } else{
                fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->databaseHandle));
                return REGISTER_ERR;
            }
        }
    } else{
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->databaseHandle));
        return REGISTER_ERR;
    }

}