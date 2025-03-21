/*                                 SUCHAI
 *                      NANOSATELLITE FLIGHT SOFTWARE
 *
 *      Copyright 2019, Carlos Gonzalez Cortes, carlgonz@uchile.cl
 *      Copyright 2019, Ignacio Ibanez Aliaga, ignacio.ibanez@usach.cl
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

#include "osScheduler.h"

/**
 * starts the scheduler of the system operating
 */
void osScheduler(os_thread* thread_id, int n_thread)
{
    printf("[INFO] Starting FreeRTOS scheduler...\n");
    vTaskStartScheduler();

    /* FIXME: Should the system restart? */
    while(1)
    {
    	printf("[ERROR] FreeRTOS scheduler stopped!\n");
    }
}
