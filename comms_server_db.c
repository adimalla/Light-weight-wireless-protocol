/**
 ******************************************************************************
 * @file    comms_server_db.c
 * @author  Aditya Mall,
 * @brief   (6314) wireless network server connected devices database source
 *              file.
 *
 *  Info
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2019 Aditya Mall, MIT License </center></h2>
 *
 * MIT License
 *
 * Copyright (c) 2019 Aditya Mall
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */



/*
 * Standard Header and API Header files
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "comms_server_db.h"



/******************************************************************************/
/*                                                                            */
/*                           API Functions                                    */
/*                                                                            */
/******************************************************************************/



/* -2 : JOINRESP_NACK, -3: JOINRESP_DUP */
table_retval_t update_client_table(client_devices_t *device_table, protocol_handle_t *protocol_server, device_config_t *server)
{
    table_retval_t return_value;

    uint8_t index = 0;
    uint8_t found = 0;

    uint16_t requested_slots = 0;

    /* Get number of slots from the payload */
    requested_slots = atoi(protocol_server->joinrequest_msg->payload);

    /* Error check */
    if(device_table == NULL || protocol_server == NULL || server == NULL )
    {
        return_value.table_retval = -1;
    }
    else if(requested_slots > COMMS_SERVER_MAX_SLOTS)
    {
        return_value.table_retval = -2;
    }
    else
    {
        /* get data from join request */
        for(index = 0; index < 5; index++)
        {
            if(memcmp(device_table[index].client_mac, protocol_server->joinrequest_msg->source_mac,6) == 0)
            {
                found = 1;

                /* Request for already present device */
                return_value.table_retval = -3;

                return_value.table_index = index;

                break;
            }
            else if(device_table[index].client_id == 0) /* Fill the next available row */
            {
                found = 0;

                /* Add new device ID to table */
                device_table[index].client_id = server->total_slots + 1;

                /* Add MAC address to table */
                memcpy(device_table[index].client_mac, protocol_server->joinrequest_msg->source_mac, 6);

                /* Update slot */
                if(requested_slots > 1)
                {
                    server->total_slots += requested_slots;
                }
                else
                {
                    server->total_slots++;
                }

                /* Add client slots to table */
                device_table[index].client_number_of_slots = requested_slots;


                /* update device count */
                server->device_count++;

                /* return client ID */
                return_value.table_index = index;

                return_value.table_retval = 0;

                break;
            }

        }
    }

    return return_value;
}



int8_t read_client_table(char *client_mac_address, int8_t *client_id, client_devices_t *device_table, int16_t table_index)
{
    int8_t func_retval;

    if(table_index < 0)
    {
        //strncpy(client_mac_address, device_table[table_index].client_mac, 6);
        *client_id = table_index;

        func_retval = -4;
    }
    else
    {
        strncpy(client_mac_address, device_table[table_index].client_mac, 6);
        *client_id = device_table[table_index].client_id;

        func_retval = 0;
    }

    return func_retval;

}






