/**
 * @file  commandRepoitory.h
 * @author Carlos Gonzalez C - carlgonz@ug.uchile.cl
 * @author Tomas Opazo T - tomas.opazo.t@gmail.com
 * @author Matias Ramirez M  - nicoram.mt@gmail.com
 * @date 2018
 * @copyright GNU GPL v3
 *
 * This header is an API to Status Repository and Data Repository:
 *  Status Repository:
 *      dat_system_var
 *
 *  Data Repository:
 *      DAT_FligthPlanBuff
 *      DAT_TeleCmdBuff
 *      DAT_PayloadVar
 *      DAT_GnrlPurpBuff
 */

#ifndef DATA_REPO_H
#define DATA_REPO_H

#include "osSemphr.h"

#include "config.h"
#include "globals.h"
#include "utils.h"

#if SCH_STORAGE_MODE
    #include "data_storage.h"
#endif

#define FP_MAX_ENTRIES          100
#define DAT_OBC_OPMODE_NORMAL   (0) ///< Normal operation
#define DAT_OBC_OPMODE_WARN     (1) ///< Fail safe operation
#define DAT_OBC_OPMODE_FAIL     (2) ///< Generalized fail operation

#define DAT_REPO_SYSTEM "dat_system"
#define DAT_WEATHER_SYSTEM "weather_system"

/**
 * System level status variables
 */
typedef struct fp_entry {
    int unixtime;               ///< time to execute in unixtime
    char* cmd;                  ///< command to execute
    char* args;                 ///< command arguments
    int executions;             ///< times to be executed
    int periodical;             ///< time to be executed again
} fp_entry_t;

/**
 * System level status variables
 */
typedef enum dat_system{
    //OBC => (C&DH subsystem)
    dat_obc_opmode=0,           ///< General operation mode
    dat_obc_lastResetSource,    ///< Last reset source
    dat_obc_hours_alive,        ///< Hours since first boot
    dat_obc_hours_without_reset,///< Hours since last reset
    dat_obc_reset_counter,      ///< Number of reset since first boot
    dat_obc_gnd_wdt,                ///< GND watchdog timer counter

    //DEP => (C&DH subsystem)
    dat_dep_ant_deployed,       ///< Is antenna deployed

    //RTC => (C&DH subsystem)
    dat_rtc_date_time,          /// RTC current unix time

    //TRX => (Communication subsystem)
    dat_trx_opmode,             ///< TRX Operation mode
    dat_trx_count_tm,           ///< number of sent TM
    dat_trx_count_tc,           ///< number of received TC
    dat_trx_last_tc,            ///< day of the last received tc

    //FLIGHT PLAN
    dat_fpl_last,              ///< Last executed flight plan
    dat_fpl_queue,             ///< Flight plan queue length

    // Add custom status variables here
    dat_balloon_phase,          ///< Balloon phase (0: (A) ascend, 1 (A1) ascend, 2 (B) stable, 3 (C) descend, 4 (C) ground.

    // Las element, do not edit
    dat_system_last_var         ///< Dummy element
}dat_system_t;

/**
 * Initializes data repositories including buffers and mutexes
 */
void dat_repo_init(void);

/**
 * Cleanup and close data repository resources
 */
void dat_repo_close(void);

/**
 * Sets a status variable value
 *
 * @param index dat_system_t. Variable to set @sa dat_system_t
 * @param value Int. Value to set
 */
void dat_set_system_var(dat_system_t index, int value);

/**
 * Returns a status variable's value
 *
 * @param index Variable to set @sa DAT_CubesatVar
 * @return Variable value
 */
int dat_get_system_var(dat_system_t index);

/**
 *
 * Get the necessary parameters to send a command and set the values in
 * the variables command, args, repeat and periodical
 *
 * @param elapsed_sec Int. time in unixtime
 * @param command Save the command name
 * @param args Save the command arguments
 * @param repeat Save the times to execute the command
 * @param periodical Save the periodical value (1 is periodical, 0 is not periodical)
 * @return 0 OK, -1 Error
 */
int dat_get_fp(int elapsed_sec, char** command, char** args, int** executions, int** periodical);

/**
 *
 * Set a command with its args, executions (how many times will be executed)
 * and periodical (seconds to be executed again) to be executed at a certain time
 *
 * @param timetodo Int. time to execute the command
 * @param command Str. command name
 * @param args Str. command arguments
 * @param repeat Int. times to execute the command
 * @param periodical Int. periodical value (in seconds)
 * @return 0 OK, -1 Error
 */
int dat_set_fp(int timetodo, char* command, char* args, int executions, int periodical);

/**
 *
 * Detelete a flight plan command that will be executed at a certain time
 *
 * @param timetodo Int. time to execute the command
 * @return 0 OK, 1 Error
 */
int dat_del_fp(int timetodo);

/**
 *
 *Restart the flight plan
 *
 * @return 0 OK, 1 Error
 */
int dat_reset_fp(void);

/**
 *
 *Show the flight plan
 *
 * @return 0 OK, 1 Error
 */
int dat_show_fp (void);

/**
 *
 * Get the system time
 *
 * @return time_t actual system time (in seconds)
 */
time_t dat_get_time(void);

/**
 *
 * Without Linux this function update a second in the system time
 *
 * @return 0 OK, 1 Error
 */
int dat_update_time(void);

/**
 *
 * Set the system time (only if not Linux)
 *
 * @param new_time Int. time to set as system time
 * @return 0 OK, 1 Error
 */
int dat_set_time(int new_time);

/**
 *
 * Show the system time in the format given
 *
 * @param command Int. format , 0 for ISO, 1 for UNIX TIME
 * @return 0 OK, 1 Error
 */
int dat_show_time(int format);







/* The following is an API to interface with the repoData cubesat fligthPlan */
//typedef enum _DAT_FligthPlanBuff{
//    dat_fpb_last_one
//}DAT_FligthPlanBuff;
//
//DispCmd dat_getFlightPlan(unsigned int index);
//int dat_setFlightPlan_cmd(unsigned int index, unsigned int cmdID);
//int dat_setFlightPlan_param(unsigned int index, int param);
//int dat_onResetFlightPlan(void);
//void dat_erase_FlightPlanBuff(void);



#endif // DATA_REPO_H

