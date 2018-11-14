/**
 * @file  taskCommunications.h
 * @author Carlos Gonzalez C - carlgonz@uchile.cl
 * @date 2018
 * @copyright GNU GPL v3
 *
 * This task implements a client that reads remote commands from TRX. Also
 * works as the CSP server to process common services and custom ports.
 *
 */

#ifndef T_COMMUNICATIONS_H
#define T_COMMUNICATIONS_H

#include "stdlib.h"

#include "os.h"
#include "csp/csp.h"
#include "csp/csp_types.h"

#include "config.h"
#include "globals.h"

#include "repoCommand.h"
#include "data_storage.h"

#define COM_GPS_NODE 5
#define COM_DPL_NODE 2
#define COM_PRS_NODE 4
#define COM_IMET_NODE 7

void taskCommunications(void *param);

#endif //T_COMMUNICATIONS_H
