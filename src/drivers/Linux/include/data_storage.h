//
// Created by carlos on 22-08-17.
//

#ifndef SCH_PERSISTENT_H
#define SCH_PERSISTENT_H

#include "utils.h"
#include <stdio.h>
#include <sqlite3.h>
#include "config.h"


/**
 * Init data storage system.
 * In this case we use SQLite, so this function open a database in file
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param file Str. File path to SQLite database
 * @return 0 OK, -1 Error
 */
int storage_init(const char *file);

/**
 * Create new table in the opened database (@relatesalso storage_init) in the
 * form (index, name, value). If the table exists do nothing. If drop is set to
 * 1 then drop an existing table and then creates an empty one.
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. Table name
 * @param drop Int. Set to 1 to drop the existing table before create one
 * @return 0 OK, -1 Error
 */
int storage_table_repo_init(char *table, int drop);

/**
 * Create new table in the opened database (@relatesalso storage_init) in the
 * form (time, command, args, repeat). If the table exists do nothing. If drop is set to
 * 1 then drop an existing table and then creates an empty one.
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param drop Int. Set to 1 to drop the existing table before create one
 * @return 0 OK, -1 Error
 */
int storage_table_flight_plan_init(int drop);

/**
 * Get an INT (integer) value from table by index
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param index Int. Value index
 * @param table Str. Table name
 * @return 0 OK, -1 Error
 */
int storage_repo_get_value_idx(int index, char *table);

/**
 * Get a INT (integer) value from table by name
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param name Str. Value name
 * @param table Str. Table name
 * @return 0 OK, -1 Error
 */
int storage_repo_get_value_str(char *name, char *table);

/**
 * Set or update the value of a INT (integer) variable by index.
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param index Int. Variable index
 * @param value Int. Value to set
 * @param table Str. Table name
 * @return 0 OK, -1 Error
 */
int storage_repo_set_value_idx(int index, int value, char *table);

/**
 * Set or update the value of a INT (integer) variable by name.
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param name Str. Variable name
 * @param value Int. Value to set
 * @param table Str. Table name
 * @return 0 OK, -1 Error
 */
int storage_repo_set_value_str(char *name, int value, char *table);

/**
 * Set or update the row of a certain time
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param timetodo Int. time to do the action
 * @param command Str. Command to set
 * @param args Str. command's arguments
 * @param repeat Int. Value of time to run the command
 * @return 0 OK, -1 Error
 */
int storage_flight_plan_set(int timetodo, char* command, char* args, int repeat, int periodical);

/**
 * Get the row of a certain time and set the values in the variables committed
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param timetodo Int. time to do the action
 * @param command Str. Command to get
 * @param args Str. command's arguments
 * @param repeat Int. Value of times to run the command
 * @return 0 OK, -1 Error
 */
int storage_flight_plan_get(int timetodo, char* command, char* args, int* repeat, int* periodical);

/**
 * Erase the row in the table in the opened database (@relatesalso storage_init) that
 * have the same timetodo.
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param timetodo Int. time to do the action
 * @return 0 OK, -1 Error
 */
int storage_flight_plan_erase(int timetodo);

/**
 * Reset the table in the opened database (@relatesalso storage_init) in the
 * form (time, command, args, repeat).
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @return 0 OK, -1 Error
 */
int storage_flight_plan_reset(void);

/**
 * Show the table in the opened database (@relatesalso storage_init) in the
 * form (time, command, args, repeat).
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @return 0 OK
 */
int storage_show_table(void);

/**
 * Close the opened database
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @return 0 OK, -1 Error
 */
int storage_close(void);

/* Second Mission specific data functions */

/**
 * Init gps payload data table
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. table name
 * @param drop Int. set to 1 to drop table and 0 to create
 *
 * @return 0 OK, -1 Error
 */
int storage_table_gps_init(char* table, int drop);

/**
 * Init pressure payload data table
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. table name
 * @param drop Int. set to 1 to drop table and 0 to create
 *
 * @return 0 OK, -1 Error
 */
int storage_table_pressure_init(char* table, int drop);

/**
 * Init deploy payload data table
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. table name
 * @param drop Int. set to 1 to drop table and 0 to create
 *
 * @return 0 OK, -1 Error
 */
int storage_table_imet_init(char* table, int drop);

/**
 * Init imet payload data table
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. table name
 * @param drop Int. set to 1 to drop table and 0 to create
 *
 * @return 0 OK, -1 Error
 */

int storage_table_deploy_init(char* table, int drop);

/**
 * Init generic payload data table with provided initialization string
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. table name
 * @param init_sql Str. sql string to initialize table
 * @param drop Int. set to 1 to drop table and 0 to create
 *
 * @return 0 OK, -1 Error
 */
int storage_table_generic_init(char* table, char* init_sql, int drop);

typedef struct gps_data {
    char timestamp[25];
    float latitude;
    float longitude;
    float height;
    float velocity_x;
    float velocity_y;
    int satellites_number;
    int mode;
    int phase;
} gps_data;

/**
 * Set a gps data frame
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. table name
 * @param data Struct. struct containing gps data
 *
 * @return 0 OK, -1 Error
 */
int storage_table_gps_set(const char* table, gps_data* data);

/**
 * Get n gps data frames
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. table name
 * @param data Struct array. array with gps data
 * @param n Int. number of frames obtained
 *
 * @return 0 OK, -1 Error
 */
int storage_table_gps_get(const char* table, gps_data data[], int n);

typedef struct prs_data {
    float pressure;
    float temperature;
    float height;
} prs_data;

/**
 * Set a pressure data frame
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. table name
 * @param data Struct. struct containing prs data
 *
 * @return 0 OK, -1 Error
 */
int storage_table_prs_set(const char* table, prs_data* data);

/**
 * Get n prs data frames
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. table name
 * @param data Struct array. array with prs data
 * @param n Int. number of frames obtained
 *
 * @return 0 OK, -1 Error
 */
int storage_table_prs_get(const char* table, prs_data data[], int n);

typedef struct dpl_data {
    int lineal_actuator;
    int servo_motor;
} dpl_data;

/**
 * Set a deploy data frame
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. table name
 * @param data Struct. struct containing dpl data
 *
 * @return 0 OK, -1 Error
 */
int storage_table_dpl_set(const char* table, dpl_data* data);

/**
 * Get n dpl data frames
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. table name
 * @param data Struct array. array with dpl data
 * @param n Int. number of frames obtained
 *
 * @return 0 OK, -1 Error
 */
int storage_table_dpl_get(const char* table, dpl_data data[], int n);

typedef struct imet_data {
    int pressure;
    int temperature;
    int humidity;
    char date[11];
    char time[9];
    int latitude;
    int longitude;
    int altitude;
    int satellites;
} imet_data;

/**
 * Set a imet data frame
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. table name
 * @param data Struct. struct containing imet data
 *
 * @return 0 OK, -1 Error
 */
int storage_table_imet_set(const char* table, imet_data* data);

/**
 * Get n imet data frames
 *
 * @note: non-reentrant function, use mutex to sync access
 *
 * @param table Str. table name
 * @param data Struct array. array with imet data
 * @param n Int. number of frames obtained
 *
 * @return 0 OK, -1 Error
 */
int storage_table_imet_get(const char* table, imet_data data[], int n);

#endif //SCH_PERSISTENT_H
