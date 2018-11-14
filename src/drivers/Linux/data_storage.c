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
        sqlite3_free(sql);
        return -1;
    }
    else
    {
        LOGD(tag, "Table %s created successfully", fp_table);
        sqlite3_free(sql);
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

int storage_flight_plan_get(int timetodo, char* command, char* args, int* executions, int* periodical)
{
    char **results;
    char *err_msg;
    int row;
    int col;

    char* sql = sqlite3_mprintf("SELECT * FROM %s WHERE time = %d", fp_table, timetodo);

    sqlite3_get_table(db, sql, &results,&row,&col,&err_msg);

    if(row==0 || col==0)
    {
        sqlite3_free(sql);
        LOGV(tag, "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        sqlite3_free_table(results);
        return -1;
    }
    else
    {
        strcpy(command, results[6]);
        strcpy(args,results[7]);
        *executions = atoi(results[8]);
        *periodical = atoi(results[9]);

        storage_flight_plan_erase(timetodo);

        if (atoi(results[9]) > 0)
            storage_flight_plan_set(timetodo+*periodical,results[6],results[7],*executions,*periodical);

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


/* Second Mission specific data functions */

int storage_table_gps_init(char* table, int drop)
{
    char * init_sql;
    init_sql= "CREATE TABLE IF NOT EXISTS %s("
            "idx INTEGER PRIMARY KEY, "
            "date_time TEXT, "
            "timestamp TEXT, "
            "latitude REAL, "
            "longitude REAL, "
            "height REAL, "
            "velocity_x REAL, "
            "velocity_y REAL, "
            "satellites_number INTEGER, "
            "mode INTEGER, "
            "phase INTEGER);";
    return storage_table_generic_init(table, init_sql, drop);
}

int storage_table_pressure_init(char* table, int drop)
{
    char * init_sql;
    init_sql= "CREATE TABLE IF NOT EXISTS %s("
            "idx INTEGER PRIMARY KEY, "
            "date_time TEXT, "
            "pressure REAL, "
            "temperature REAL, "
            "height REAL);";
    return storage_table_generic_init(table, init_sql, drop);
}

int storage_table_deploy_init(char* table, int drop)
{
    char * init_sql;
    init_sql= "CREATE TABLE IF NOT EXISTS %s("
            "idx INTEGER PRIMARY KEY, "
            "date_time TEXT, "
            "lineal_actuator INTEGER, "
            "servo_motor INTEGER);";
    return storage_table_generic_init(table, init_sql, drop);
}

int storage_table_imet_init(char* table, int drop)
{
    char * init_sql;
    init_sql= "CREATE TABLE IF NOT EXISTS %s("
              "idx INTEGER PRIMARY KEY, "
              "date_time TEXT, "
              "pressure INTEGER, "
              "temperature INTEGER, "
              "humidity INTEGER, "
              "date TEXT, "
              "time TEXT, "
              "latitude INTEGER, "
              "longitude INTEGER, "
              "altitude INTEGER, "
              "satellites INTEGER);";
    return storage_table_generic_init(table, init_sql, drop);
}

int storage_table_generic_init(char* table, char* init_sql, int drop)
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
        sql = sqlite3_mprintf(init_sql, table);
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

int storage_table_gps_set(const char* table, gps_data* data)
{
    char *err_msg;
    int rc;

    char *sql = sqlite3_mprintf(
            "INSERT OR REPLACE INTO %s "
                    "(date_time, timestamp, latitude, longitude, height, velocity_x, velocity_y, satellites_number, mode, phase)\n "
                    "VALUES (datetime(\"now\"), \"%s\", %f, %f, %f, %f, %f, %d, %d, %d);",
            table, data->timestamp, data->latitude, data->longitude, data->height, data->velocity_x, data->velocity_y, data->satellites_number, data->mode, data->phase);

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
        LOGI(tag, "Inserted  gps data");
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return 0;
    }
}

int storage_table_gps_get(const char* table, gps_data data[], int n)
{
    char **results;
    char *err_msg;

    char *sql = sqlite3_mprintf("SELECT * FROM %s ORDER BY idx DESC LIMIT %d", table, n);

    int row;
    int col;

    // execute statement
    sqlite3_get_table(db, sql, &results, &row, &col, &err_msg);

    if(row==0 || col==0)
    {
        LOGI(tag, "GPS table empty");
        return 0;
    }
    else
    {
        LOGI(tag, "GPS table")
        int i;
        for (i = 0; i < (col*row)+col; i++)
        {
            printf("%s\t", results[i]);
            if ((i + 1) % col == 0)
                printf("\n");
        }

        for (i = 0; i < row; i++)
        {
            // maybe memcpy?
            strcpy(data[i].timestamp, results[(i*col)+col+2]);
            data[i].latitude =  atof(results[(i*col)+col+3]);
            data[i].longitude = atof(results[(i*col)+col+4]);
            data[i].height = atof(results[(i*col)+col+5]);
            data[i].velocity_x = atof(results[(i*col)+col+6]);
            data[i].velocity_y = atof(results[(i*col)+col+7]);
            data[i].satellites_number = atoi(results[(i*col)+col+8]);
            data[i].mode = atoi(results[(i*col)+col+9]);
            data[i].phase = atoi(results[(i*col)+col+10]);
        }
    }
    return 0;
}

int storage_table_prs_set(const char* table, prs_data* data)
{
    char *err_msg;
    int rc;

    char *sql = sqlite3_mprintf(
            "INSERT OR REPLACE INTO %s "
                    "(date_time, pressure, temperature, height)\n "
                    "VALUES (datetime(\"now\"), %f, %f, %f);",
            table, data->pressure, data->temperature, data->height);

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
        LOGI(tag, "Inserted  prs data");
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return 0;
    }
}

int storage_table_prs_get(const char* table, prs_data data[], int n)
{
    char **results;
    char *err_msg;

    char *sql = sqlite3_mprintf("SELECT * FROM %s ORDER BY idx DESC LIMIT %d", table, n);

    int row;
    int col;

    // execute statement
    sqlite3_get_table(db, sql, &results, &row, &col, &err_msg);

    if(row==0 || col==0)
    {
        LOGI(tag, "PRS table empty");
        return 0;
    }
    else
    {
        LOGI(tag, "PRS table")
        int i;
        for (i = 0; i < (col*row)+col; i++)
        {
            printf("%s\t", results[i]);
            if ((i + 1) % col == 0)
                printf("\n");
        }

        for (i = 0; i < row; i++)
        {
            data[i].pressure =  atof(results[(i*col)+col+2]);
            data[i].temperature = atof(results[(i*col)+col+3]);
            data[i].height = atof(results[(i*col)+col+4]);
        }
    }
    return 0;
}

int storage_table_dpl_set(const char* table, dpl_data* data)
{
    char *err_msg;
    int rc;

    char *sql = sqlite3_mprintf(
            "INSERT OR REPLACE INTO %s "
                    "(date_time, lineal_actuator, servo_motor)\n "
                    "VALUES (datetime(\"now\"), %d, %d);",
            table, data->lineal_actuator, data->servo_motor);

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
        LOGI(tag, "Inserted  dpl data");
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return 0;
    }
}

int storage_table_dpl_get(const char* table, dpl_data data[], int n)
{
    char **results;
    char *err_msg;

    char *sql = sqlite3_mprintf("SELECT * FROM %s ORDER BY idx DESC LIMIT %d", table, n);

    int row;
    int col;

    // execute statement
    sqlite3_get_table(db, sql, &results, &row, &col, &err_msg);

    if(row==0 || col==0)
    {
        LOGI(tag, "DPL table empty");
        return 0;
    }
    else
    {
        LOGI(tag, "DPL table")
        int i;
        for (i = 0; i < (col*row)+col; i++)
        {
            printf("%s\t", results[i]);
            if ((i + 1) % col == 0)
                printf("\n");
        }

        for (i = 0; i < row; i++)
        {
            data[i].lineal_actuator =  atof(results[(i*col)+col+2]);
            data[i].servo_motor= atof(results[(i*col)+col+3]);
        }
    }
    return 0;
}

int storage_table_imet_set(const char* table, imet_data* data)
{
    char *err_msg;
    int rc;

    char *sql = sqlite3_mprintf(
            "INSERT OR REPLACE INTO %s "
            "(date_time, pressure, temperature, humidity, date, time, latitude, longitude, altitude, satellites)\n "
            "VALUES (datetime(\"now\"), %d, %d, %d, \"%s\", \"%s\", %d, %d, %d, %d);",
            table, data->pressure, data->temperature, data->humidity, data->date, data->time, data->latitude, data->longitude, data->altitude, data->satellites);

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
        LOGI(tag, "Inserted  gps data");
        sqlite3_free(err_msg);
        sqlite3_free(sql);
        return 0;
    }
}

int storage_table_imet_get(const char* table, imet_data data[], int n)
{
    char **results;
    char *err_msg;

    char *sql = sqlite3_mprintf("SELECT * FROM %s ORDER BY idx DESC LIMIT %d", table, n);

    int row;
    int col;

    // execute statement
    sqlite3_get_table(db, sql, &results, &row, &col, &err_msg);

    if(row==0 || col==0)
    {
        LOGI(tag, "IMET table empty");
        return 0;
    }
    else
    {
        LOGI(tag, "IMET table")
        int i;
        for (i = 0; i < (col*row)+col; i++)
        {
            printf("%s\t", results[i]);
            if ((i + 1) % col == 0)
                printf("\n");
        }

        for (i = 0; i < row; i++)
        {
            int pressure;
            int temperature;
            int humidity;
            char date[11];
            char time[9];
            int latitude;
            int longitude;
            int altitude;
            int satellites;

            data[i].pressure=  atof(results[(i*col)+col+2]);
            data[i].temperature = atof(results[(i*col)+col+3]);
            data[i].humidity = atof(results[(i*col)+col+4]);
            strcpy(data[i].date, results[(i*col)+col+5]);
            strcpy(data[i].time, results[(i*col)+col+6]);
            data[i].latitude = atoi(results[(i*col)+col+7]);
            data[i].longitude = atoi(results[(i*col)+col+8]);
            data[i].satellites = atoi(results[(i*col)+col+9]);
        }
    }
    return 0;
}




