/*                                 SUCHAI
 *                      NANOSATELLITE FLIGHT SOFTWARE
 *
 *      Copyright 2019, Carlos Gonzalez Cortes, carlgonz@uchile.cl
 *      Copyright 2019, Tomas Opazo Toro, tomas.opazo.t@gmail.com
 *      Copyright 2019, Camilo Rojas Milla, camrojas@uchile.cl
 *      Copyright 2019, Matias Ramirez Martinez, nicoram.mt@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "repoData.h"

static const char *tag = "repoData";
char* table = "flightPlan";
#ifdef AVR32
time_t sec = 0;
#endif


#if SCH_STORAGE_MODE == 0
#if SCH_STORAGE_TRIPLE_WR == 1
        int DAT_SYSTEM_VAR_BUFF[dat_system_last_var*3];
    #else
        int DAT_SYSTEM_VAR_BUFF[dat_system_last_var];
    #endif
    fp_entry_t data_base [SCH_FP_MAX_ENTRIES];
#endif

struct map data_map[last_sensor] = {
        {"temp_data",      (uint16_t) (sizeof(temp_data_t)),     dat_drp_temp, dat_drp_ack_temp, "%u %f %f %f",                   "timestamp obc_temp_1 obc_temp_2 obc_temp_3"},
        { "ads_data",      (uint16_t) (sizeof(ads_data_t)),      dat_drp_ads,  dat_drp_ack_ads,  "%u %f %f %f %f %f %f",          "timestamp acc_x acc_y acc_z mag_x mag_y mag_z"},
        { "eps_data",      (uint16_t) (sizeof(eps_data_t)),      dat_drp_eps,  dat_drp_ack_eps,  "%u %u %u %u %d %d %d %d %d %d", "timestamp cursun cursys vbatt temp1 temp2 temp3 temp4 temp5 temp6"},
        { "langmuir_data", (uint16_t) (sizeof(langmuir_data_t)), dat_drp_lang, dat_drp_ack_lang, "%u %f %f %f %d",                "timestamp sweep_voltage plasma_voltage plasma_temperature particles_counter"}
};

void initialize_payload_vars(void){
    int i =0;
    for(i=0; i< last_sensor; ++i) {
        if(dat_get_system_var(data_map[i].sys_index) == -1) {
            dat_set_system_var(data_map[i].sys_index, 0);
            dat_set_system_var(data_map[i].sys_ack, 0);
        }
    }
}

void dat_repo_init(void)
{
    // Init repository mutex
    if(osSemaphoreCreate(&repo_data_sem) != CSP_SEMAPHORE_OK)
    {
        LOGE(tag, "Unable to create system status repository mutex");
    }

    if(osSemaphoreCreate(&repo_data_fp_sem) != CSP_SEMAPHORE_OK)
    {
        LOGE(tag, "Unable to create flight plan repository mutex");
    }

    LOGD(tag, "Initializing data repositories buffers...")
    /* TODO: Setup external memories */
#if (SCH_STORAGE_MODE == 0)
    {
        // Reset variables (we do not have persistent storage here)
        int index;
        for(index=0; index<dat_system_last_var; index++)
        {
            dat_set_system_var((dat_system_t)index, INT_MAX);
        }

        //Init internal flight plan table
        int i;
        for(i=0;i<SCH_FP_MAX_ENTRIES;i++)
        {
            data_base[i].unixtime = 0;
            data_base[i].cmd = NULL;
            data_base[i].args = NULL;
            data_base[i].executions = 0;
            data_base[i].periodical = 0;
        }
    }
#elif (SCH_STORAGE_MODE > 0)
    {
        //Init storage system
        int rc;
        rc = storage_init(SCH_STORAGE_FILE);
        assertf(rc==0, tag, "Unable to create non-volatile data repository");

        //Init system repo
        rc = storage_table_repo_init(DAT_REPO_SYSTEM, 0);
        assertf(rc==0, tag, "Unable to create system variables repository");

        //Init payloads repo
        rc = storage_table_payload_init(0);
        assertf(rc==0, tag, "Unable to create payload repo");

        //Init system flight plan table
        rc=storage_table_flight_plan_init(0);
        assertf(rc==0, tag, "Unable to create flight plan table");
    }
#endif

//    /* TODO: Initialize custom variables */
//    LOGD(tag, "Initializing system variables values...")
////    dat_set_system_var(dat_obc_hrs_alive, 0);
//    dat_set_system_var(dat_obc_hrs_wo_reset, 0);
//    dat_set_system_var(dat_obc_reset_counter, dat_get_system_var(dat_obc_reset_counter) + 1);
//    dat_set_system_var(dat_obc_sw_wdt, 0);  // Reset the gnd wdt on boot
//
//#if (SCH_STORAGE_MODE > 0)
//    initialize_payload_vars();
//#endif
}

void dat_repo_close(void)
{
#if SCH_STORAGE_MODE != 0
    {
        storage_close();
    }
#endif
}

/**
 * Function for testing triple writing.
 *
 * Should do the same as @c dat_set_system_var , but with only one system status repo.
 *
 * @param index Enum index of the field to set
 * @param value Integer value to set the variable to
 */
void _dat_set_system_var(dat_system_t index, int value)
{
    //Enter critical zone
    osSemaphoreTake(&repo_data_sem, portMAX_DELAY);

    //Uses internal memory
#if SCH_STORAGE_MODE == 0
    DAT_SYSTEM_VAR_BUFF[index] = value;
    //Uses external memory
#else
    storage_repo_set_value_idx(index, value, DAT_REPO_SYSTEM);
#endif

    //Exit critical zone
    osSemaphoreGiven(&repo_data_sem);
}

/**
 * Function for testing triple writing.
 *
 * Should do the same as @c dat_get_system_var , but with only one system status repo.
 *
 * @param index Enum index of the field to get
 * @return The field's value
 */
int _dat_get_system_var(dat_system_t index)
{
    int value = 0;

    //Enter critical zone
    osSemaphoreTake(&repo_data_sem, portMAX_DELAY);

    //Use internal (volatile) memory
#if SCH_STORAGE_MODE == 0
    value = DAT_SYSTEM_VAR_BUFF[index];
    //Uses external (non-volatile) memory
#else
    value = storage_repo_get_value_idx(index, DAT_REPO_SYSTEM);
#endif

    //Exit critical zone
    osSemaphoreGiven(&repo_data_sem);

    return value;
}

void dat_set_system_var(dat_system_t index, int value)
{
    //Enter critical zone
    osSemaphoreTake(&repo_data_sem, portMAX_DELAY);

    //Uses internal memory
#if SCH_STORAGE_MODE == 0
    DAT_SYSTEM_VAR_BUFF[index] = value;
        //Uses tripled writing
        #if SCH_STORAGE_TRIPLE_WR == 1
            DAT_SYSTEM_VAR_BUFF[index + dat_system_last_var] = value;
            DAT_SYSTEM_VAR_BUFF[index + dat_system_last_var * 2] = value;
        #endif
    //Uses external memory
#else
    storage_repo_set_value_idx(index, value, DAT_REPO_SYSTEM);
    //Uses tripled writing
#if SCH_STORAGE_TRIPLE_WR == 1
    storage_repo_set_value_idx(index + dat_system_last_var, value, DAT_REPO_SYSTEM);
    storage_repo_set_value_idx(index + dat_system_last_var * 2, value, DAT_REPO_SYSTEM);
#endif
#endif

    //Exit critical zone
    osSemaphoreGiven(&repo_data_sem);
}

int dat_get_system_var(dat_system_t index)
{
    int value_1 = 0;
    int value_2 = 0;
    int value_3 = 0;

    //Enter critical zone
    osSemaphoreTake(&repo_data_sem, portMAX_DELAY);

    //Use internal (volatile) memory
#if SCH_STORAGE_MODE == 0
    value_1 = DAT_SYSTEM_VAR_BUFF[index];
        //Uses tripled writing
        #if SCH_STORAGE_TRIPLE_WR == 1
            value_2 = DAT_SYSTEM_VAR_BUFF[index + dat_system_last_var];
            value_3 = DAT_SYSTEM_VAR_BUFF[index + dat_system_last_var * 2];
        #endif
    //Uses external (non-volatile) memory
#else
    value_1 = storage_repo_get_value_idx(index, DAT_REPO_SYSTEM);
    //Uses tripled writing
#if SCH_STORAGE_TRIPLE_WR == 1
    value_2 = storage_repo_get_value_idx(index + dat_system_last_var, DAT_REPO_SYSTEM);
    value_3 = storage_repo_get_value_idx(index + dat_system_last_var * 2, DAT_REPO_SYSTEM);
#endif
#endif

    //Exit critical zone
    osSemaphoreGiven(&repo_data_sem);
#if SCH_STORAGE_TRIPLE_WR == 1
    //Compare value and its copies
    if (value_1 == value_2 || value_1 == value_3)
    {
        return value_1;
    }
    else if (value_2 == value_3)
    {
        return value_2;
    }
    else
    {
        LOGE(tag, "Unable to get a correct value for index %d", index);
        return value_1;
    }
#else
    return value_1;
#endif
}

void dat_status_to_struct(dat_status_t *status)
{
    assert(status != NULL);
    DAT_CPY_SYSTEM_VAR(status, dat_obc_opmode);        ///< General operation mode
    DAT_CPY_SYSTEM_VAR(status, dat_obc_last_reset);    ///< Last reset source
    DAT_CPY_SYSTEM_VAR(status, dat_obc_hrs_alive);     ///< Hours since first boot
    DAT_CPY_SYSTEM_VAR(status, dat_obc_hrs_wo_reset);  ///< Hours since last reset
    DAT_CPY_SYSTEM_VAR(status, dat_obc_reset_counter); ///< Number of reset since first boot
    DAT_CPY_SYSTEM_VAR(status, dat_obc_sw_wdt);        ///< Software watchdog timer counter
    DAT_CPY_SYSTEM_VAR_F(status, dat_obc_temp_1);        ///< Temperature value of the first sensor
    DAT_CPY_SYSTEM_VAR_F(status, dat_obc_temp_2);        ///< Temperature value of the second sensor
    DAT_CPY_SYSTEM_VAR_F(status, dat_obc_temp_3);        ///< Temperature value of the gyroscope

    DAT_CPY_SYSTEM_VAR(status, dat_dep_deployed);      ///< Was the satellite deployed?
    DAT_CPY_SYSTEM_VAR(status, dat_dep_ant_deployed);  ///< Was the antenna deployed?
    DAT_CPY_SYSTEM_VAR(status, dat_dep_date_time);     ///< Deployment unix time

    DAT_CPY_SYSTEM_VAR(status, dat_rtc_date_time);     /// RTC current unix time

    DAT_CPY_SYSTEM_VAR(status, dat_com_count_tm);      ///< number of TM sent
    DAT_CPY_SYSTEM_VAR(status, dat_com_count_tc);      ///< number of received TC
    DAT_CPY_SYSTEM_VAR(status, dat_com_last_tc);       ///< Unix time of the last received tc
    DAT_CPY_SYSTEM_VAR(status, dat_com_freq);          ///< Frequency [Hz]
    DAT_CPY_SYSTEM_VAR(status, dat_com_tx_pwr);        ///< TX power (0: 25dBm, 1: 27dBm, 2: 28dBm, 3: 30dBm)
    DAT_CPY_SYSTEM_VAR(status, dat_com_baud);          ///< Baudrate [bps]
    DAT_CPY_SYSTEM_VAR(status, dat_com_mode);          ///< Framing mode (1: RAW, 2: ASM, 3: HDLC, 4: Viterbi, 5: GOLAY, 6: AX25)
    DAT_CPY_SYSTEM_VAR(status, dat_com_bcn_period);    ///< Number of seconds between beacon packets

    DAT_CPY_SYSTEM_VAR(status, dat_fpl_last);          ///< Last executed flight plan (unix time)
    DAT_CPY_SYSTEM_VAR(status, dat_fpl_queue);         ///< Flight plan queue length

    DAT_CPY_SYSTEM_VAR_F(status, dat_ads_acc_x);         ///< Gyroscope acceleration value along the x axis
    DAT_CPY_SYSTEM_VAR_F(status, dat_ads_acc_y);         ///< Gyroscope acceleration value along the y axis
    DAT_CPY_SYSTEM_VAR_F(status, dat_ads_acc_z);         ///< Gyroscope acceleration value along the z axis
    DAT_CPY_SYSTEM_VAR_F(status, dat_ads_mag_x);         ///< Magnetometer x axis
    DAT_CPY_SYSTEM_VAR_F(status, dat_ads_mag_y);         ///< Magnetometer y axis
    DAT_CPY_SYSTEM_VAR_F(status, dat_ads_mag_z);         ///< Magnetometer z axis

    DAT_CPY_SYSTEM_VAR(status, dat_eps_vbatt);         ///< Voltage of battery [mV]
    DAT_CPY_SYSTEM_VAR(status, dat_eps_cur_sun);       ///< Current from boost converters [mA]
    DAT_CPY_SYSTEM_VAR(status, dat_eps_cur_sys);       ///< Current out of battery [mA]
    DAT_CPY_SYSTEM_VAR(status, dat_eps_temp_bat0);     ///< Battery temperature sensor

    DAT_CPY_SYSTEM_VAR(status, dat_drp_temp);
    DAT_CPY_SYSTEM_VAR(status, dat_drp_ads);
    DAT_CPY_SYSTEM_VAR(status, dat_drp_eps);
    DAT_CPY_SYSTEM_VAR(status, dat_drp_lang);

    DAT_CPY_SYSTEM_VAR(status, dat_drp_ack_temp);
    DAT_CPY_SYSTEM_VAR(status, dat_drp_ack_ads);
    DAT_CPY_SYSTEM_VAR(status, dat_drp_ack_eps);
    DAT_CPY_SYSTEM_VAR(status, dat_drp_ack_lang);
}

void dat_print_status(dat_status_t *status)
{
    DAT_PRINT_SYSTEM_VAR(status, dat_obc_opmode);        ///< General operation mode
    DAT_PRINT_SYSTEM_VAR(status, dat_obc_last_reset);    ///< Last reset source
    DAT_PRINT_SYSTEM_VAR(status, dat_obc_hrs_alive);     ///< Hours since first boot
    DAT_PRINT_SYSTEM_VAR(status, dat_obc_hrs_wo_reset);  ///< Hours since last reset
    DAT_PRINT_SYSTEM_VAR(status, dat_obc_reset_counter); ///< Number of reset since first boot
    DAT_PRINT_SYSTEM_VAR(status, dat_obc_sw_wdt);        ///< Software watchdog timer counter
    DAT_PRINT_SYSTEM_VAR_F(status, dat_obc_temp_1);      ///< Temperature value of the first sensor
    DAT_PRINT_SYSTEM_VAR_F(status, dat_obc_temp_2);      ///< Temperature value of the second sensor
    DAT_PRINT_SYSTEM_VAR_F(status, dat_obc_temp_3);      ///< Temperature value of the gyroscope

    DAT_PRINT_SYSTEM_VAR(status, dat_dep_deployed);      ///< Was the satellite deployed?
    DAT_PRINT_SYSTEM_VAR(status, dat_dep_ant_deployed);  ///< Was the antenna deployed?
    DAT_PRINT_SYSTEM_VAR(status, dat_dep_date_time);     ///< Deployment unix time

    DAT_PRINT_SYSTEM_VAR(status, dat_rtc_date_time);     /// RTC current unix time

    DAT_PRINT_SYSTEM_VAR(status, dat_com_count_tm);      ///< number of TM sent
    DAT_PRINT_SYSTEM_VAR(status, dat_com_count_tc);      ///< number of received TC
    DAT_PRINT_SYSTEM_VAR(status, dat_com_last_tc);       ///< Unix time of the last received tc
    DAT_PRINT_SYSTEM_VAR(status, dat_com_freq);          ///< Frequency [Hz]
    DAT_PRINT_SYSTEM_VAR(status, dat_com_tx_pwr);        ///< TX power (0: 25dBm, 1: 27dBm, 2: 28dBm, 3: 30dBm)
    DAT_PRINT_SYSTEM_VAR(status, dat_com_baud);          ///< Baudrate [bps]
    DAT_PRINT_SYSTEM_VAR(status, dat_com_mode);          ///< Framing mode (1: RAW, 2: ASM, 3: HDLC, 4: Viterbi, 5: GOLAY, 6: AX25)
    DAT_PRINT_SYSTEM_VAR(status, dat_com_bcn_period);    ///< Number of seconds between beacon packets

    DAT_PRINT_SYSTEM_VAR(status, dat_fpl_last);          ///< Last executed flight plan (unix time)
    DAT_PRINT_SYSTEM_VAR(status, dat_fpl_queue);         ///< Flight plan queue length

    DAT_PRINT_SYSTEM_VAR_F(status, dat_ads_acc_x);         ///< Gyroscope acceleration value along the x axis
    DAT_PRINT_SYSTEM_VAR_F(status, dat_ads_acc_y);         ///< Gyroscope acceleration value along the y axis
    DAT_PRINT_SYSTEM_VAR_F(status, dat_ads_acc_z);         ///< Gyroscope acceleration value along the z axis
    DAT_PRINT_SYSTEM_VAR_F(status, dat_ads_mag_x);         ///< Magnetometer x axis
    DAT_PRINT_SYSTEM_VAR_F(status, dat_ads_mag_y);         ///< Magnetometer y axis
    DAT_PRINT_SYSTEM_VAR_F(status, dat_ads_mag_z);         ///< Magnetometer z axis

    DAT_PRINT_SYSTEM_VAR(status, dat_eps_vbatt);         ///< Voltage of battery [mV]
    DAT_PRINT_SYSTEM_VAR(status, dat_eps_cur_sun);       ///< Current from boost converters [mA]
    DAT_PRINT_SYSTEM_VAR(status, dat_eps_cur_sys);       ///< Current out of battery [mA]
    DAT_PRINT_SYSTEM_VAR(status, dat_eps_temp_bat0);     ///< Battery temperature sensor

    DAT_PRINT_SYSTEM_VAR(status, dat_drp_temp);
    DAT_PRINT_SYSTEM_VAR(status, dat_drp_ads);
    DAT_PRINT_SYSTEM_VAR(status, dat_drp_eps);
    DAT_PRINT_SYSTEM_VAR(status, dat_drp_lang);

    DAT_PRINT_SYSTEM_VAR(status, dat_drp_ack_temp);
    DAT_PRINT_SYSTEM_VAR(status, dat_drp_ack_ads);
    DAT_PRINT_SYSTEM_VAR(status, dat_drp_ack_eps);
    DAT_PRINT_SYSTEM_VAR(status, dat_drp_ack_lang);
}

#if SCH_STORAGE_MODE == 0
static int _dat_set_fp_async(int timetodo, char* command, char* args, int executions, int periodical)
{
    int i;
    for(i = 0;i < SCH_FP_MAX_ENTRIES;i++)
    {
        if(data_base[i].unixtime == 0)
        {
            data_base[i].unixtime = timetodo;
            data_base[i].executions = executions;
            data_base[i].periodical = periodical;

            data_base[i].cmd = malloc(sizeof(char)*50);
            data_base[i].args = malloc(sizeof(char)*50);

            strcpy(data_base[i].cmd, command);
            strcpy(data_base[i].args,args);

            return 0;
        }
    }

    return 1;
}

static int _dat_del_fp_async(int timetodo)
{
    int i;
    for(i = 0;i < SCH_FP_MAX_ENTRIES;i++)
    {
        if(timetodo == data_base[i].unixtime)
        {
            data_base[i].unixtime = 0;
            data_base[i].executions = 0;
            data_base[i].periodical = 0;
            free(data_base[i].args);
            free(data_base[i].cmd);
            return 0;
        }
    }
    return 1;
}
#endif

int dat_set_fp(int timetodo, char* command, char* args, int executions, int periodical)
{

    osSemaphoreTake(&repo_data_fp_sem, portMAX_DELAY);
    //Enter critical zone
#if SCH_STORAGE_MODE == 0
    //TODO : agregar signal de segment para responder falla
    int rc = _dat_set_fp_async(timetodo, command, args, executions, periodical);
#else
    int rc = storage_flight_plan_set(timetodo, command, args, executions, periodical);
#endif
    //Exit critical zone
    osSemaphoreGiven(&repo_data_fp_sem);
    return rc;
}

int dat_get_fp(int elapsed_sec, char* command, char* args, int* executions, int* periodical)
{
    int rc;
    osSemaphoreTake(&repo_data_fp_sem, portMAX_DELAY);
    //Enter critical zone
#if SCH_STORAGE_MODE == 0
    int i;
    rc = -1;  // not found by default
    for(i = 0;i < SCH_FP_MAX_ENTRIES;i++)
    {
        if(elapsed_sec == data_base[i].unixtime)
        {
            strcpy(command, data_base[i].cmd);
            strcpy(args,data_base[i].args);
            *executions = data_base[i].executions;
            *periodical = data_base[i].periodical;

            if (data_base[i].periodical > 0)
                _dat_set_fp_async(elapsed_sec+*periodical,data_base[i].cmd,data_base[i].args,*executions,*periodical);

            _dat_del_fp_async(elapsed_sec);
            rc = 0;  // found, then break!
            break;
        }
    }
#else
    rc =storage_flight_plan_get(elapsed_sec, command, args, executions, periodical);
#endif
    //Exit critical zone
    osSemaphoreGiven(&repo_data_fp_sem);

    return rc;
}

int dat_del_fp(int timetodo)
{

    osSemaphoreTake(&repo_data_fp_sem, portMAX_DELAY);
    //Enter critical zone
#if SCH_STORAGE_MODE ==0
    int rc = _dat_del_fp_async(timetodo);
#else
    int rc = storage_flight_plan_erase(timetodo);
#endif
    //Exit critical zone
    osSemaphoreGiven(&repo_data_fp_sem);

    return rc;
}

int dat_reset_fp(void)
{
    int rc;
    osSemaphoreTake(&repo_data_fp_sem, portMAX_DELAY);
    //Enter critical zone
#if SCH_STORAGE_MODE == 0
    int i;
    for(i=0;i<SCH_FP_MAX_ENTRIES;i++)
    {
        data_base[i].unixtime = 0;
        data_base[i].cmd = NULL;
        data_base[i].args = NULL;
        data_base[i].executions = 0;
        data_base[i].periodical = 0;
    }
    rc = 0;
#else
    rc = storage_table_flight_plan_init(1);
#endif
    //Exit critical zone
    osSemaphoreGiven(&repo_data_fp_sem);
    return rc;
}

int dat_show_fp (void)
{
    int rc;
    osSemaphoreTake(&repo_data_fp_sem, portMAX_DELAY);
    //Enter critical zone
#if SCH_STORAGE_MODE ==0
    int cont = 0;
    int i;
    for(i = 0;i < SCH_FP_MAX_ENTRIES; i++)
    {
        if(data_base[i].unixtime!=0)
        {
            if(cont == 0)
            {
                printf("When\tCommand\tArguments\tExecutions\tPeriodical\n");
                cont++;
            }
            time_t time_to_show = data_base[i].unixtime;
            printf("%s\t%s\t%s\t%d\t%d\n",ctime(&time_to_show),data_base[i].cmd,data_base[i].args,data_base[i].executions,data_base[i].periodical);
        }
    }
    if(cont == 0)
    {
        LOGI(tag, "Flight plan table empty");
    }
    rc = 0;
#else
    rc = storage_show_table();
#endif
    //Exit critical zone
    osSemaphoreGiven(&repo_data_fp_sem);
    return rc;
}

time_t dat_get_time(void)
{
#ifdef AVR32
    return sec;
#else
    return time(NULL);
#endif
}

int dat_update_time(void)
{
#ifdef AVR32
    sec++;
    return 0;
#else
    return 0;
#endif
}

int dat_set_time(int new_time)
{
#if defined AVR32
    sec = (time_t)new_time;
    return 0;
#elif defined ESP32
    return 0;
#elif defined NANOMIND
    timestamp_t timestamp = {(uint32_t)new_time, 0};
    clock_set_time(&timestamp);
    return 0;
#else
    // TODO: This needs to be tested on a raspberry LINUX system, to see if the sudo call asks for permissions or not

    size_t command_length = 28;

    time_t new_time_typed = (time_t)new_time;
    char* arg = ctime(&new_time_typed);

    size_t arg_length = strlen(arg);

    char command[sizeof(char)*(command_length+arg_length+1)];
    command[command_length+arg_length] = '\0';

    strncpy(command, "sudo hwclock --set --date '", command_length-1);
    strncpy(command+command_length-1, arg, arg_length);
    strncpy(command+command_length+arg_length-1, "'", 1);

    int rc = system(command);

    if (rc == -1)
    {
        LOGE(tag, "Failed attempt at creating a child process for setting the machine clock");
        return 1;
    }
    else if (rc == 127)
    {
        LOGE(tag, "Failed attempt at starting a shell for setting machine clock");
        return 1;
    }
    else
    {
        return rc != 0 ? 1 : 0;
    }
#endif
}

int dat_show_time(int format)
{
#ifdef AVR32
    time_t time_to_show = dat_get_time();
#else
    time_t time_to_show = time(NULL);
#endif

    if(format == 0)
    {
        printf("%s\n",ctime(&time_to_show));
        return 0;
    }
    else if(format == 1)
    {
        printf("%u\n", (unsigned int)time_to_show);
        return 0;
    }
    else
    {
        return 1;
    }
}

int dat_add_payload_sample(void* data, int payload)
{
    int ret;

    int index = dat_get_system_var(data_map[payload].sys_index);
    LOGI(tag, "Adding data for payload %d in index %d", payload, index);

    //Enter critical zone
    osSemaphoreTake(&repo_data_sem, portMAX_DELAY);

#if defined(LINUX) || defined(NANOMIND)
    ret = storage_set_payload_data(index, data, payload);
#else
    ret=0;
#endif
    //Exit critical zone
    osSemaphoreGiven(&repo_data_sem);

    // Update address
    if(ret==0) {
        dat_set_system_var(data_map[payload].sys_index, index+1);
        return index+1;
    } else {
        LOGE(tag, "Couldn't set data payload %d", payload);
        return -1;
    }
}


int dat_get_recent_payload_sample(void* data, int payload, int delay)
{
    //TODO: Change variable name from delay to offset??
    int ret;

    int index = dat_get_system_var(data_map[payload].sys_index);
    LOGV(tag, "Obtaining data of payload %d, in index %d, sys_var: %d", payload, index,data_map[payload].sys_index );

    //Enter critical zone
    osSemaphoreTake(&repo_data_sem, portMAX_DELAY);
    //TODO: Is this conditional required?
#if defined(LINUX) || defined(NANOMIND)
    if(index-1-delay >= 0) {
        ret = storage_get_payload_data(index-1-delay, data, payload);
    }
    else {
        //FIXME: "Asked for too large offset (%d) on payload (%d)
        LOGE(tag, "Asked for too great of a delay when requesting payload %d on delay %d", payload, delay);
        ret = -1;
    }
#else
    ret=0;
#endif
    //Exit critical zone
    osSemaphoreGiven(&repo_data_sem);
    return ret;
}

int dat_delete_memory_sections(void)
{
    int ret;
    // Resetting memory system vars
    for(int i = 0; i < last_sensor; ++i)
    {
        dat_set_system_var(data_map[i].sys_index, 0);
    }
    //Enter critical zone
    osSemaphoreTake(&repo_data_sem, portMAX_DELAY);
#ifdef NANOMIND
    ret = storage_delete_memory_sections();
#else
    ret=0;
#endif
    //Exit critical zone
    osSemaphoreGiven(&repo_data_sem);
#if SCH_FP_ENABLED
    storage_flight_plan_reset();
#endif
    return ret;
}


int get_payloads_tokens(char** tok_sym, char** tok_var, char* order, char* var_names, int i)
{
    const char s[2] = " ";
    tok_sym[0] = strtok(order, s);

    int j=0;
    while(tok_sym[j] != NULL) {
        j++;
        tok_sym[j] = strtok(NULL, s);
    }

    tok_var[0] = strtok(var_names, s);

    j=0;
    while(tok_var[j] != NULL) {
        j++;
        tok_var[j] = strtok(NULL, s);
    }
    return j;
}

void get_value_string(char* ret_string, char* c_type, char* buff)
{
    if(strcmp(c_type, "%f") == 0) {
        sprintf(ret_string, " %f", *((float*)buff));
    }
    else if(strcmp(c_type, "%d") == 0) {
        sprintf(ret_string, " %d", *((int*)buff));
    }
    else if(strcmp(c_type, "%u") == 0) {
        sprintf(ret_string, " %u", *((unsigned int*)buff));
    }
}

int get_sizeof_type(char* c_type)
{
    if(strcmp(c_type, "%f") == 0) {
        return sizeof(float);
    }
    else if(strcmp(c_type, "%d") == 0) {
        return sizeof(int);
    } else if(strcmp(c_type, "%u") == 0) {
        return sizeof(int);
    } else {
        return -1;
    }
}

int dat_print_payload_struct(void* data, unsigned int payload)
{
    //FIXME: Buffers allocated in stack with magical numbers! Use defined values
    //FIXME: Use malloc to allocate large buffers, here 1280  bytes allocated!
    //FIXME: Task stack size: 5*256 = 1280 bytes!

    char* tok_sym[30];
    char* tok_var[30];
    char *order = (char *)malloc(50);
    strcpy(order, data_map[payload].data_order);
    char *var_names = (char *)malloc(200);
    strcpy(var_names, data_map[payload].var_names);
    int nparams = get_payloads_tokens(tok_sym, tok_var, order, var_names, (int)payload);

    char *values = (char *)malloc(500);
    char *names = (char *)malloc(500);
    strcpy(names, "");
    strcpy(values, "");

    int j;
    for(j=0; j < nparams; ++j) {
        int param_size = get_sizeof_type(tok_sym[j]);
        char buff[param_size];
        memcpy(buff, data+(j*param_size), param_size);

        char name[20];
        sprintf(name, " %s", tok_var[j]);
        strcat(names, name);

        char val[20];
        get_value_string(val, tok_sym[j], buff);
        strcat(values, val);

        if(j != nparams-1){
            strcat(names, ",");
            strcat(values, ",");
        }
    }
    strcat(names, ":");
    printf("%s %s\n", names, values);

    free(order);
    free(var_names);
    free(values);
    free(names);

    return 0;
}