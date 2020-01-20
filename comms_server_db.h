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
#include "comms_protocol.h"


/******************************************************************************/
/*                                                                            */
/*                  Data Structures Server Devices Table                      */
/*                                                                            */
/******************************************************************************/



typedef struct table_return_values
{
    uint8_t table_index;
    int8_t table_retval;

}table_retval_t;



typedef struct client_devices
{
    uint8_t client_id;
    char    client_mac[6];
    uint8_t client_number_of_slots;
    uint8_t client_state;

    char    client_ip_address[4];

}client_devices_t;






int8_t read_client_table(char *client_mac_address, int8_t *client_id, client_devices_t *device_table, int16_t table_index);



table_retval_t update_server_device_table(client_devices_t *device_table, char *client_mac_address, uint8_t requested_slots, device_config_t *server);



#endif /* COMMS_SERVER_DB_H_ */
