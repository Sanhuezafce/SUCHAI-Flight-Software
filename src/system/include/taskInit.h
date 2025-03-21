/**
 * @file  taskInit.h
 * @author Matias Ramirez  - nicoram.mt@gmail.com
 * @author Carlos Gonzalez C - carlgonz@uchile.cl
 * @date 2019
 * @copyright GNU GPL v3
 *
 * This task implements a listener, that sends commands at periodical times.
 */

#ifndef SUCHAI_FLIGHT_SOFTWARE_TASKINIT_H
#define SUCHAI_FLIGHT_SOFTWARE_TASKINIT_H

#include "utils.h"
#include "config.h"
#include "init.h"
#include "drivers.h"

#include "osThread.h"
#include "osQueue.h"

/* Task includes */
#include "taskConsole.h"
#if SCH_HK_ENABLED
#include "taskHousekeeping.h"
#endif
#if SCH_COMM_ENABLE
#include "taskCommunications.h"
#endif
#if SCH_FP_ENABLED
#include "taskFlightPlan.h"
#endif

void taskInit(void *param);
void init_communications(void);
void init_routines(void);

#endif //SUCHAI_FLIGHT_SOFTWARE_TASKINIT_H
