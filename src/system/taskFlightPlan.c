/*                                 SUCHAI
 *                      NANOSATELLITE FLIGHT SOFTWARE
 *
 *      Copyright 2019 Matias Ramirez Martinez, nicoram.mt@gmail.com
 *      Copyright 2019, Carlos Gonzalez Cortes, carlgonz@uchile.cl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "taskFlightPlan.h"

static const char *tag = "FlightPlan"; 

void taskFlightPlan(void *param)
{

    LOGI(tag, "Started");

    portTick delay_ms = 1000;          //Task period in [ms]
    portTick xLastWakeTime = osTaskGetTickCount();

    time_t elapsed_sec;   // Seconds counter

    while(1)
    {
        osTaskDelayUntil(&xLastWakeTime, delay_ms); //Suspend task

#ifdef AVR32
        elapsed_sec = dat_get_time();
#else
        elapsed_sec = time(NULL);
#endif

        char command[SCH_CMD_MAX_STR_PARAMS];
        char args[SCH_CMD_MAX_STR_PARAMS];
        int executions;
        int periodical;

        int rc = dat_get_fp((int)elapsed_sec, command, args, &executions, &periodical);

        if(rc == -1){
            continue;
        }

        char* fixed_args = fix_fmt(args);
        int i;
        for(i=0; i < executions; i++)
        {
            cmd_t *new_cmd = cmd_get_str(command);
            cmd_add_params_str(new_cmd, fixed_args);

            LOGD(tag, "Command: %s", command);
            LOGD(tag, "Arguments: %s", fixed_args);
            LOGD(tag, "Executions: %d", executions);
            LOGD(tag, "Periodical: %d", periodical);
            dat_set_system_var(dat_fpl_last, (int) elapsed_sec);
            cmd_send(new_cmd);
        }
    }
}

int date_to_unixtime(int day, int month, int year, int hour, int min, int sec)
{
    struct tm str_time;
    time_t unixtime;

    str_time.tm_mday = day;
    str_time.tm_mon = month-1;
    str_time.tm_year = year-1900;
    str_time.tm_hour = hour;
    str_time.tm_min = min;
    str_time.tm_sec = sec;

    unixtime = mktime(&str_time);

    return (int)unixtime;
}