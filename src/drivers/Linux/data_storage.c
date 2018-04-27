//
// Created by carlos on 22-08-17.
//

#include "data_storage.h"

static const char *tag = "data_storage";

static sqlite3 *db = NULL;
char* fp_table = "flightPlan";

static int dummy_callback(void *data, int argc, char **argv, char **names);

int storage_init(const char *file)
{
    if(db != NULL)
    {
        LOGW(tag, "Database already open, closing it");
        sqlite3_close(db);
    }

    // Open database
    if(sqlite3_open(file, &db) != SQLITE_OK)
    {
        LOGE(tag, "Can't open database: %s", sqlite3_errmsg(db));
        return -1;
    }
    else
    {
        LOGD(tag, "Opened database successfully");
        return 0;
    }
}

int storage_table_repo_init(char* table, int drop)
{
    char *err_msg;
    char *sql;
    int rc;

    /* Drop table if selected */
    if(drop)
    {
        sql = sqlite3_mprintf("DROP TABLE %s", table);
        rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

        if (rc != SQLITE_OK )
        {
            LOGE(tag, "Failed to drop table %s. Error: %s. SQL: %s", table, err_msg, sql);
            sqlite3_free(err_msg);
            sqlite3_free(sql);
            return -1;
        }
        else
        {
            LOGD(tag, "Table %s drop successfully", table);
            sqlite3_free(sql);
        }
    }

    sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s("
                                        "idx INTEGER PRIMARY KEY, "
                                        "name TEXT UNIQUE, "
                                        "value INT);",
                                table);

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK )
    {
        LOGE(tag, "Failed to crate table %s. Error: %s. SQL: %s", table, err_msg, sql);
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return -1;
    }
    else
    {
        LOGD(tag, "Table %s created successfully", table);
        sqlite3_free(sql);
        return 0;
    }
}

int storage_table_flight_plan_init(int drop)
{

    char* err_msg;
    char* sql;
    int rc;

    /* Drop table if selected */
    if (drop)
    {
        sql = sqlite3_mprintf("DROP TABLE IF EXISTS %s", fp_table);
        rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

        if (rc != SQLITE_OK )
        {
            LOGE(tag, "Failed to drop table %s. Error: %s. SQL: %s", fp_table, err_msg, sql);
            sqlite3_free(err_msg);
            sqlite3_free(sql);
            return -1;
        }
        else
        {
            LOGD(tag, "Table %s drop successfully", fp_table);
            sqlite3_free(sql);
        }
    }

    sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s("
                                  "time int PRIMARY KEY , "
                                  "command text, "
                                  "args text , "
                                  "executions int , "
                                  "periodical int );",
                          fp_table);

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK )
    {
        LOGE(tag, "Failed to crate table %s. Error: %s. SQL: %s", fp_table, err_msg, sql);

        return -1;
    }
    else
    {
        LOGD(tag, "Table %s created successfully", fp_table);

        return 0;
    }
}

int storage_repo_get_value_idx(int index, char *table)
{
    sqlite3_stmt* stmt = NULL;
    char *sql = sqlite3_mprintf("SELECT value FROM %s WHERE idx=\"%d\";", table, index);

    // execute statement
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if(rc != SQLITE_OK)
    {
        LOGE(tag, "Selecting data from DB Failed (rc=%d)", rc);
        return -1;
    }

    // fetch only one row's status
    rc = sqlite3_step(stmt);
    int value = -1;
    if(rc == SQLITE_ROW)
        value = sqlite3_column_int(stmt, 0);
    else
        LOGE(tag, "Some error encountered (rc=%d)", rc);

    sqlite3_finalize(stmt);
    sqlite3_free(sql);
    return value;
}

int storage_repo_get_value_str(char *name, char *table)
{
    sqlite3_stmt* stmt = NULL;
    char *sql = sqlite3_mprintf("SELECT value FROM %s WHERE name=\"%s\";", table, name);

    // execute statement
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if(rc != 0)
    {
        LOGE(tag, "Selecting data from DB Failed (rc=%d)", rc);
        return -1;
    }

    // fetch only one row's status
    rc = sqlite3_step(stmt);
    int value = -1;
    if(rc == SQLITE_ROW)
        value = sqlite3_column_int(stmt, 0);
    else
        LOGE(tag, "Some error encountered (rc=%d)", rc);

    sqlite3_finalize(stmt);
    sqlite3_free(sql);
    return value;
}

int storage_repo_set_value_idx(int index, int value, char *table)
{
    char *err_msg;
    char *sql = sqlite3_mprintf("INSERT OR REPLACE INTO %s (idx, name, value) "
                                        "VALUES ("
                                        "%d, "
                                        "(SELECT name FROM %s WHERE idx = \"%d\"), "
                                        "%d);",
                                table, index, table, index, value);

    /* Execute SQL statement */
    int rc = sqlite3_exec(db, sql, dummy_callback, 0, &err_msg);

    if( rc != SQLITE_OK )
    {
        LOGE(tag, "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return -1;
    }
    else
    {
        LOGV(tag, "Inserted %d to %d in %s", value, index, table);
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return 0;
    }
}

int storage_repo_set_value_str(char *name, int value, char *table)
{
    char *err_msg;
    char *sql = sqlite3_mprintf("INSERT OR REPLACE INTO %s (idx, name, value) "
                                        "VALUES ("
                                            "(SELECT idx FROM %s WHERE name = \"%s\"), "
                                            "%s, "
                                            "%d);",
                                table, table, name, name, value);

    /* Execute SQL statement */
    int rc = sqlite3_exec(db, sql, dummy_callback, 0, &err_msg);

    if( rc != SQLITE_OK )
    {
        LOGE(tag, "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return -1;
    }
    else
    {
        LOGV(tag, "Inserted %d to %s in %s", value, name, table);
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return 0;
    }
}

int storage_flight_plan_set(int timetodo, char* command, char* args, int executions, int periodical)
{
    char *err_msg;
    char *sql = sqlite3_mprintf(
            "INSERT OR REPLACE INTO %s (time, command, args, executions, periodical)\n VALUES (%d, \"%s\", \"%s\", %d, %d);",
            fp_table, timetodo, command, args, executions, periodical);

    /* Execute SQL statement */
    int rc = sqlite3_exec(db, sql, dummy_callback, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        LOGE(tag, "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return -1;
    }
    else
    {
        LOGV(tag, "Inserted (%d, %s, %s, %d, %d) in %s", timetodo, command, args, executions, periodical, fp_table);
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return 0;
    }
}

int storage_flight_plan_get(int timetodo, char** command, char** args, int** executions, int** periodical)
{
    char **results;
    char *err_msg;
    int row;
    int col;
    char* sql = sqlite3_mprintf("SELECT * FROM %s WHERE time = %d", fp_table, timetodo);


    // execute statement
    sqlite3_get_table(db, sql, &results,&row,&col,&err_msg);

    if(row==0 || col==0)
    {
        return -1;
    }
    else
    {
        strcpy(*command, results[6]);
        strcpy(*args,results[7]);
        **executions = atoi(results[8]);
        **periodical = atoi(results[9]);

        storage_flight_plan_erase(timetodo);

        if (atoi(results[9]) > 0)
            storage_flight_plan_set(timetodo+**periodical,results[6],results[7],**executions,**periodical);

        sqlite3_free(sql);
        return 0;
    }
}

int storage_flight_plan_erase(int timetodo)
{

    char *err_msg;
    char *sql = sqlite3_mprintf("DELETE FROM %s\n WHERE time = %d", fp_table, timetodo);

    /* Execute SQL statement */
    int rc = sqlite3_exec(db, sql, dummy_callback, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        LOGE(tag, "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return -1;
    }
    else
    {
        LOGV(tag, "Command in time %d, table %s was deleted", timetodo, fp_table);
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return 0;
    }
}

int storage_flight_plan_reset(void)
{
    return storage_table_flight_plan_init(1);
}

int storage_show_table (void) {
    char **results;
    char *err_msg;
    int row;
    int col;
    char *sql = sqlite3_mprintf("SELECT * FROM %s", fp_table);

    // execute statement
    sqlite3_get_table(db, sql, &results,&row,&col,&err_msg);

    if(row==0 || col==0)
    {
        LOGI(tag, "Flight plan table empty");
        return 0;
    }
    else
    {
        LOGI(tag, "Flight plan table")
        int i;
        for (i = 0; i < (col*row + 5); i++)
        {
            if (i%col == 0 && i!=0)
            {
                time_t timef = atoi(results[i]);
                printf("%s\t",ctime(&timef));
                continue;
            }
            printf("%s\t", results[i]);
            if ((i + 1) % col == 0)
                printf("\n");
        }
    }
    return 0;
}

int storage_close(void)
{
    if(db != NULL)
    {
        LOGD(tag, "Closing database");
        sqlite3_close(db);
        db = NULL;
        return 0;
    }
    else
    {
        LOGW(tag, "Attempting to close a NULL pointer database");
        return -1;
    }
}

static int dummy_callback(void *data, int argc, char **argv, char **names)
{
    return 0;
}

// Baloon weather functions

int storage_table_weather_init(char* table, int drop)
{
    char *err_msg;
    char *sql;
    int rc;

    if(drop)
    {
        sql = sqlite3_mprintf("DROP TABLE %s", table);
        rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

        if (rc != SQLITE_OK )
        {
            LOGE(tag, "Failed to drop table %s. Error: %s. SQL: %s", table, err_msg, sql);
            sqlite3_free(err_msg);
            sqlite3_free(sql);
            return -1;
        }
        else
        {
            LOGD(tag, "Table %s drop successfully", table);
            sqlite3_free(sql);
        }
    }
    else
    {
        sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s("
                                      "idx INTEGER PRIMARY KEY, "
                                      "temp1 REAL, "
                                      "temp2 REAL, "
                                      "press1 REAL, "
                                      "height REAL, "
                                      "imu1 REAL, "
                                      "imu2 REAL, "
                                      "imu3 REAL, "
                                      "gps_lat REAL, "
                                      "gps_lon REAL, "
                                      "gps_height REAL, "
                                      "gps_cur REAL, "
                                      "gps_v REAL, "
                                      "gps_HH INTEGER, "
                                      "gps_MM INTEGER, "
                                      "gps_SS INTEGER, "
                                      "gps_SAT INTEGER, "
                                      "rssi INTEGER);", table);

        rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

        if (rc != SQLITE_OK )
        {
            LOGE(tag, "Failed to crate table %s. Error: %s. SQL: %s", table, err_msg, sql);
            sqlite3_free(err_msg);
            sqlite3_free(sql);
            return -1;
        }
        else
        {
            LOGD(tag, "Table %s created successfully", table);
            sqlite3_free(sql);
            return 0;
        }

    }

}

int storage_get_weather_data(const char* table, weather_data* data, int n)
{
    char **results;
    char *err_msg;

    char *sql = sqlite3_mprintf("SELECT * FROM %s ORDER BY idx DESC LIMIT %d", table, n);

    int row;
    int col;

    // execute statement
    sqlite3_get_table(db, sql, &results,&row,&col,&err_msg);

    if(row==0 || col==0)
    {
        LOGI(tag, "Weather table empty");
        return 0;
    }
    else
    {
        LOGI(tag, "Weather table")
        int i;
        for (i = 0; i < (col*row)+col; i++)
        {
//            if (i%col == 0 && i!=0)
//            {
//                time_t timef = atoi(results[i]);
//                printf("%s\t",ctime(&timef));
//                continue;
//            }
            printf("%s\t", results[i]);
            if ((i + 1) % col == 0)
                printf("\n");
        }
    }
    return 0;
}

int storage_set_weather_data(const char* table, weather_data* data)
{
    char *err_msg;
    int rc;

    char *sql = sqlite3_mprintf(
            "INSERT OR REPLACE INTO %s "
                    "(temp1, temp2, press1, height, imu1, imu2, imu3, gps_lat, gps_lon, gps_height, gps_cur, gps_v,"
                    " gps_HH, gps_MM, gps_SS, gps_SAT, rssi)\n "
                    "VALUES (%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %d, %d, %d, %d, %d);",
            table, data->temp1, data->temp2, data->press1, data->height, data->imu1, data->imu2, data->imu3,
            data->gps_lat, data->gps_lon, data->gps_height, data->gps_cur, data->gps_v, data->gps_HH, data->gps_MM,
            data->gps_SS, data->gps_SAT, data->rssi);

    rc = sqlite3_exec(db, sql, dummy_callback, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        LOGE(tag, "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return -1;
    }
    else
    {
        LOGI(tag, "Inserted  weather data");
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return 0;
    }
}
