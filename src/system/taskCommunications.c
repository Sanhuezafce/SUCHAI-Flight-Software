/*                                 SUCHAI
 *                      NANOSATELLITE FLIGHT SOFTWARE
 *
 *      Copyright 2018, Carlos Gonzalez Cortes, carlgonz@uchile.cl
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

//#include <csp/csp_types.h>
#include "taskCommunications.h"

static const char *tag = "Communications";

static void com_receive_tc(csp_packet_t *packet);
static void com_receive_tm(csp_packet_t *packet, int src);
static void com_receive_cmd(csp_packet_t *packet);

void taskCommunications(void *param)
{
    LOGI(tag, "Started");
    int rc;

    /* Pointer to current connection, packet and socket */
    csp_conn_t *conn;
    csp_packet_t *packet;
    csp_packet_t *rep_ok_tmp;
    csp_packet_t *rep_ok;

    csp_socket_t *sock = csp_socket(CSP_SO_NONE);
    if((rc = csp_bind(sock, CSP_ANY)) != CSP_ERR_NONE)
    {
        LOGE(tag, "Error biding socket (%d)!", rc)
        return;
    }
    if((rc = csp_listen(sock, 5)) != CSP_ERR_NONE)
    {
        LOGE(tag, "Error listening to socket (%d)", rc)
        return;
    }

    rep_ok_tmp = csp_buffer_get(1);
    memset(rep_ok_tmp->data, 200, 1);
    rep_ok_tmp->length = 1;

    int count_tc = 0;

    while(1)
    {
        /* CSP SERVER */
        /* Wait for connection, 1000 ms timeout */
        if((conn = csp_accept(sock, 1000)) == NULL)
            continue; /* Try again later */

        /* Read packets. Timeout is 500 ms */
        while ((packet = csp_read(conn, 500)) != NULL)
        {
            printf("packet received with destiny: %d \n", csp_conn_dport(conn) );
            switch (csp_conn_dport(conn))
            {
                case SCH_TRX_PORT_TC:
                    /* Process incoming TC */
                    printf("Telemcommand obtained \n");
                    com_receive_tc(packet);
                    csp_buffer_free(packet);
                    // Create a response packet and send
//                    rep_ok = csp_buffer_clone(rep_ok_tmp);
                    rep_ok = csp_buffer_get(1);
                    rep_ok->data[0] = 200;
                    rep_ok->length = 1;
                    csp_send(conn, rep_ok, 1000);
                    break;

                case SCH_TRX_PORT_TM:
                    /* TODO: Process incoming TM */
                    printf("Telemetry obtained \n");
                    int source = csp_conn_src(conn);
                    printf("Source node is %d \n", source);
                    com_receive_tm(packet, source);
                    csp_buffer_free(packet);
                    // Create a response packet and send
                    rep_ok = csp_buffer_clone(rep_ok_tmp);
                    csp_send(conn, rep_ok, 1000);
                    break;

                case SCH_TRX_PORT_RPT:
                    // Digital repeater port, resend the received packet
                    printf("Repeat obtained \n");
                    if(csp_conn_dst(conn) == SCH_COMM_ADDRESS)
                    {
                        rc = csp_sendto(CSP_PRIO_NORM, CSP_BROADCAST_ADDR,
                                        SCH_TRX_PORT_RPT, SCH_TRX_PORT_RPT,
                                        CSP_O_NONE, packet, 1000);
                        if (rc != 0)
                            csp_buffer_free(packet); // Free the packet in case of errors
                    }
                    // If i am receiving a broadcast packet just print
                    else
                    {
                        LOGI(tag, "RPT: %s", (char *)(packet->data));
                        csp_buffer_free(packet);
                    }
                    break;

                case SCH_TRX_PORT_CMD:
                    /* Command port, executes console commands */
                    com_receive_cmd(packet);
                    csp_buffer_free(packet);
                    // Create a response packet and send
                    //rep_ok = csp_buffer_clone(rep_ok_tmp);
                    rep_ok = csp_buffer_get(1);
                    rep_ok->data[0] = 200;
                    rep_ok->length = 1;
                    csp_send(conn, rep_ok, 1000);
                    break;

                default:
                    count_tc++;
                    dat_set_system_var(dat_com_count_tc, count_tc);
                    dat_set_system_var(dat_com_last_tc, (int) time(NULL));
                    /* Let the service handler reply pings, buffer use, etc. */
                    csp_service_handler(conn, packet);
                    break;
            }
        }

        /* Close current connection, and handle next */
        csp_close(conn);
    }
}

/**
 * Parse TC frames and generates corresponding commands
 *
 * @param packet A csp buffer containing a null terminated string with the
 *               format <command> [parameters]
 */
static void com_receive_tc(csp_packet_t *packet)
{
    /* TODO: this function should receive several (cmd,args) pairs */
    /* TODO: check com_receive_cmd implementation */

    // Make sure the buffer is a null terminated string
    packet->data[packet->length] = '\0';
    cmd_t *new_cmd = cmd_parse_from_str((char *)(packet->data));

    // Send command to execution if not null
    if(new_cmd != NULL)
        cmd_send(new_cmd);
}

/**
 * Parse tc frame as console commands and execute the commands
 *
 * @param packet A csp buffer containing a null terminated string with the
 *               format <command> [parameters]
 */
static void com_receive_cmd(csp_packet_t *packet)
{
    // Make sure the buffer is a null terminated string
    packet->data[packet->length] = '\0';
    cmd_t *new_cmd = cmd_parse_from_str((char *)(packet->data));

    // Send command to execution if not null
    if(new_cmd != NULL)
        cmd_send(new_cmd);
}

/**
 * Parse TM frames
 *
 * @param packet A csp buffer containing a null terminated string with the
 *               format <command> [parameters]
 */
static void com_receive_tm(csp_packet_t *packet, int src)
{
    // Make sure the buffer is a null terminated string
    packet->data[packet->length] = '\0';
    printf("Parsing Telemetry Data \n");
    printf("%s \n", (char*)packet->data);
    int ok = 0;

    if(src == COM_GPS_NODE) {
        char timestamp[25];
        float latitude;
        float longitude;
        float height;
        float velocity_x;
        float velocity_y;
        int satellites_number;
        int mode;
        int phase = dat_get_system_var(dat_balloon_phase);      // current phase
    //    memset(timestamp, '\0', (size_t)25+1);

        // Scan a command and parameter string: <command> [parameters]
        ok = sscanf((char*)packet->data, "%s %f %f %f %f %f %d %d %d", timestamp, &latitude, &longitude, &height, &velocity_x, &velocity_y, &satellites_number, &mode, &phase);

        gps_data data;
        strcpy(data.timestamp, timestamp);
        data.latitude = latitude;
        data.longitude = longitude;
        data.height = height;
        data.velocity_x = velocity_x;
        data.velocity_y = velocity_y;
        data.satellites_number = satellites_number;
        data.mode = mode;
        data.phase = phase;

    //    printf("timestamp is: %s \n", data.timestamp);
    //    printf("latitud is: %f \n", data.latitude);
    //    printf("longitude is: %f \n", data.longitude);
    //    printf("heigth is: %f \n", data.height);
    //    printf("velocity_x is: %f \n", data.velocity_x);
    //    printf("veclocity_y is: %f \n", data.velocity_y);
    //    printf("sat number is: %d \n", data.satellites_number);
    //    printf("mode is: %d \n", data.mode);

        storage_table_gps_set(DAT_GPS_TABLE, &data);
    } else if (src == COM_DPL_NODE) {
        //dpl
        int lineal_actuator;
        int servo_motor;

        // Scan a command and parameter string: <command> [parameters]
        ok = sscanf((char*)packet->data, "%d %d", &lineal_actuator, &servo_motor);

        dpl_data data;
        data.lineal_actuator = lineal_actuator;
        data.servo_motor = servo_motor;

        storage_table_dpl_set(DAT_DPL_TABLE, &data);

    } else if (src == COM_PRS_NODE) {
        //prs
        float pressure;
        float temperature;
        float height;

        // Scan a command and parameter string: <command> [parameters]
        ok = sscanf((char*)packet->data, "%f %f %f", &pressure, &temperature, &height);

        prs_data data;
        printf("pressure: %f", data.pressure);
        data.pressure = pressure;
        data.temperature = temperature;
        data.height = height;

        storage_table_prs_set(DAT_PRS_TABLE, &data);
    } else if (src == COM_IMET_NODE) {
        int pressure;
        int temperature;
        int humidity;
        char date[11];
        char time[9];
        int latitude;
        int longitude;
        int altitude;
        int satellites;

        // Scan a command and parameter string: <command> [parameters]
        ok = sscanf((char *) packet->data, "%d %d %d %s %s %d %d %d %d", &pressure, &temperature, &humidity, date,
                    time, &latitude, &longitude, &altitude, &satellites);

        imet_data data;
        data.pressure = pressure;
        data.temperature= temperature;
        data.humidity = humidity;
        strcpy(data.date, date);
        strcpy(data.time, time);
        data.latitude = latitude;
        data.longitude = longitude;
        data.altitude = altitude;
        data.satellites = satellites;

        storage_table_imet_set(DAT_IMET_TABLE, &data);
    }
}