/**
 ******************************************************************************
 * @file    wi_network.c
 * @author  Aditya Mall,
 * @brief   (6314) wireless network type source file
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

#include "wi_network.h"
#include "network_protocol_configs.h"



#define WI_NETWORK_SYNC_MESSAGE       1




/******************************************************************************/
/*                                                                            */
/*                  Private Data Structures for wireless network              */
/*                                                                            */
/******************************************************************************/


typedef struct wi_network_header
{
    uint8_t message_status    : 4;  /*!< (LSB)*/
    uint8_t message_type      : 4;  /*!< (MSB)*/
    uint8_t message_length;         /*!< */
    uint8_t message_checksum;       /*!< */

}wi_header_t;



struct sync_packet
{
    char        preamble[WI_NET_PREAMBLE_LENGTH];  /*!< */
    wi_header_t fixed_header;                      /*!< */
    uint16_t    network_id;                        /*!< */
    uint8_t     message_slot_number;               /*!< */
    uint16_t    slot_time;                         /*!< */
    uint8_t     access_slot;                       /*!< */
    char        payload[WI_NET_PAYLOAD_LENGTH];    /*!< */

};




/********************************************************
 * @brief  Function to calculate network message checksum
 * @param  data   : Message data
 * @param  offset : Starting offset for message data
 * @param  size   : Length of message
 * @retval int8_t : Error value
 ********************************************************/
int8_t wi_network_checksum(char *data, uint8_t offset, uint8_t size)
{
    int8_t i = 0;
    uint8_t checksum = 0;

    for(i = offset; i < size; i++)
    {
        checksum = data[i] + ((checksum & 0xFF) + ((checksum >> 8) & 0xFF)) ;
    }

    return checksum;
}




/******************************************************************************/
/*                                                                            */
/*                              API Functions                                 */
/*                                                                            */
/******************************************************************************/




/***********************************************************************
 * @brief  Function to get sync message data
 * @param  *client_device   : reference to client device config structure
 * @param  *message_payload : message payload from sync message
 * @param  network          : network handle structure
 * @retval int8_t           : error: -1, success: 0
 ***********************************************************************/
int8_t get_sync_data(device_config_t *client_device, char *message_payload ,access_control_t network)
{
    int8_t func_retval;

    if(client_device == NULL || network.sync_message == NULL || message_payload == NULL)
    {
        func_retval = -1;
    }
    else
    {
        /* Get network id */
        client_device->device_network_id = network.sync_message->network_id;

        /* Get Slot Number */
        client_device->network_access_slot = network.sync_message->access_slot;

        /* get Slot time */
        client_device->device_slot_time = network.sync_message->slot_time;

        /* Get Payload Data */


        func_retval = 0;
    }

    return func_retval;
}




/***********************************************************************
 * @brief  Function to configure sync message
 * @param  *network         : reference to network handle structure
 * @param  *message_payload : message payload from syncmessage
 * @param  network          : network handle structure
 * @retval int8_t           : error: 0, success: length of message
 ***********************************************************************/
uint8_t wi_network_sync_message(access_control_t *network, uint16_t network_id, uint16_t slot_time, char *payload)
{
    uint8_t func_retval    = 0;
    uint8_t payload_length = 0;
    uint8_t message_length = 0;
    uint8_t payload_index  = 0;

    /* Check parameter error */
    if( network == NULL || slot_time > MAX_SLOT_TIME || payload == NULL)
    {
        func_retval = 0;
    }
    else
    {
        payload_length = strlen(payload);

        /* check length error */
        if(payload_length > COMMS_PAYLOAD_LENGTH - COMMS_TERMINATOR_LENGTH )
        {
            func_retval = 0;
        }
        else
        {
            network->sync_message->preamble[0] = (PREAMBLE_SYNC >> 8) & 0xFF;
            network->sync_message->preamble[1] = (PREAMBLE_SYNC >> 0) & 0xFF;

            network->sync_message->fixed_header.message_type = WI_NETWORK_SYNC_MESSAGE;

            network->sync_message->message_slot_number = COMMS_SERVER_SLOTNUM;

            network->sync_message->network_id = network_id;

            network->sync_message->access_slot = COMMS_ACCESS_SLOTNUM;

            network->sync_message->slot_time = slot_time;

            /* Add payload message */
            strncpy(network->sync_message->payload, payload, payload_length);

            payload_index = payload_length;

            /* Add message terminator */
            strncpy(network->sync_message->payload + payload_index, COMMS_MESSAGE_TERMINATOR, COMMS_TERMINATOR_LENGTH);


            /* Calculate remaining length */
            network->sync_message->fixed_header.message_length = COMMS_SLOTNUM_SIZE + COMMS_NETWORK_ID_SIZE + COMMS_ACCESS_SLOT_SIZE + \
                    COMMS_SLOT_TIME_SIZE + payload_length + COMMS_TERMINATOR_LENGTH;

            message_length = network->sync_message->fixed_header.message_length + COMMS_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH;

            /* Calculate checksum */
            network->sync_message->fixed_header.message_checksum = wi_network_checksum((char*)network->sync_message, 5 ,message_length);

            func_retval = message_length;

        }
    }

    return func_retval;
}

