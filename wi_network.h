/**
 ******************************************************************************
 * @file    wi_network.h
 * @author  Aditya Mall,
 * @brief   (6314) wireless network type header file
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


#ifndef WI_NETWORK_H_
#define WI_NETWORK_H_



/*
 * Standard Header and API Header files
 */
#include <stdint.h>



/******************************************************************************/
/*                                                                            */
/*                            Macro Defines                                   */
/*                                                                            */
/******************************************************************************/

#pragma pack(1)


#define WI_NET_PREAMBLE_LENGTH      2
#define WI_NET_PAYLOAD_LENGTH       20
#define WI_NET_MACADDR_LENGTH       6
#define WI_NET_MESSAGE_BUFFER_SIZE  32


/******************************************************************************/
/*                                                                            */
/*                  Data Structures for wireless network                      */
/*                                                                            */
/******************************************************************************/


/* Wireless network SYNC Message structure Declaration */
typedef struct sync_packet sync_packet_t;


/* */
typedef struct device_config
{
    char      device_mac[WI_NET_MACADDR_LENGTH];  /*!< */
    uint16_t  device_network_id;                  /*!< */
    uint8_t   network_access_slot;                /*!< */
    uint8_t   device_slot_number;                 /*!< */
    uint16_t  device_slot_time;                   /*!< */
    uint8_t   total_slots;                        /*!< */
    uint8_t   network_joined;                     /*!< */
    uint8_t   device_count;                       /*!< */

}device_config_t;






/* */
typedef struct access_control
{
    sync_packet_t  *sync_message;  /*!< */

}access_control_t;




/******************************************************************************/
/*                                                                            */
/*                       API Function Prototypes                              */
/*                                                                            */
/******************************************************************************/



/*********************************************************
 * @brief  Function to calculate network message checksum
 * @param  data   : Message data
 * @param  offset : Starting offset for message data
 * @param  size   : Length of message
 * @retval int8_t : Error value
 *********************************************************/
int8_t wi_network_checksum(char *data, uint8_t offset, uint8_t size);




/************************************************************************
 * @brief  Function to get sync message data
 * @param  *client_device   : reference to client device config structure
 * @param  *message_payload : message payload from syncmessage
 * @param  network          : network handle structure
 * @retval int8_t           : error -1, success: 0
 ***********************************************************************/
int8_t get_sync_data(device_config_t *client_device, char *message_payload ,access_control_t network);




/***********************************************************************
 * @brief  Function to configure sync message
 * @param  *client_device   : pointer to client device config structure
 * @param  *message_payload : message payload from syncmessage
 * @param  network          : network handle structure
 * @retval int8_t           : error: 0, success: length of message
 ***********************************************************************/
uint8_t wi_network_sync_message(access_control_t *server_device, uint16_t network_id, uint16_t slot_time, char *payload);







#endif /* WI_NETWORK_H_ */
