/**
 ******************************************************************************
 * @file    comms_server_db.h
 * @author  Aditya Mall,
 * @brief   (6314) wireless network server connected devices database header
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

#ifndef COMMS_SERVER_DB_H_
#define COMMS_SERVER_DB_H_




/*
 * Standard Header and API Header files
 */
#include "comms_network.h"


/******************************************************************************/
/*                                                                            */
/*                  Data Structures Server Devices Table                      */
/*                                                                            */
/******************************************************************************/


#define CLIENT_TABLE_SIZE 5


/* table return values structure */
typedef struct table_return_values
{
    uint8_t table_index;
    int8_t table_retval;

}table_retval_t;


typedef struct _client_states
{
    uint8_t qos        : 1;
    uint8_t keep_alive : 1;
    uint8_t reserved   : 6;

}client_states_t;


/* client device table structure */
typedef struct _client_devices
{
    uint8_t         client_id;
    char            client_mac[6];
    uint8_t         client_number_of_slots;
    client_states_t client_states;
    char            client_ip_address[4];

    uint8_t client_table_lock;

}client_devices_t;





/****************************************************************
 * @brief  Client device table contructor function
 * @retval int8_t : error = -2 JOINRESP_NACK, -3: JOINRESP_DUP,
 ***************************************************************/
client_devices_t* create_server_device_table(void);



/*****************************************************************************
 * @brief  Function write to client device table
 * @param  *device_table       : reference to the device table
 * @param  *client_mac_address : client mac address
 * @param  requested_slots     : requested slots by the client
 * @param  server              : reference to the protocol handle structure
 * @retval int8_t              : error: -2 : JOINRESP_NACK, -3: JOINRESP_DUP,
 *                               success: table current index
 *****************************************************************************/
table_retval_t update_server_device_table(client_devices_t *device_table, char *client_mac_address, uint8_t requested_slots, device_config_t *server);



/*****************************************************************
 * @brief  Function read from index value of the device table
 * @param  *device_table       : reference to the device table
 * @param  *client_mac_address : client mac address
 * @param  *client_id          : reference to client ID variable
 * @param  table index         : table index value
 * @retval int8_t              : error = -4, success = 0
 ****************************************************************/
int8_t read_client_table(client_devices_t *device_table, char *client_mac_address, int8_t *client_id,  int16_t table_index);



/*******************************************************************
 * @brief  Function to find device in client device table
 * @param  *device_table       : reference to the device table
 * @param  *client_id          : reference to client ID variable
 * @param  *client_mac_address : client mac address
 * @param  search_mode         : search by client id or mac address
 * @retval int8_t              : error = 0, success = 1
 *******************************************************************/
int8_t find_client_device(client_devices_t *device_table, uint8_t *client_id, char *client_mac_address, uint8_t search_mode);



#endif /* COMMS_SERVER_DB_H_ */
