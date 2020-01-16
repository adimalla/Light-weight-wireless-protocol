/**
 ******************************************************************************
 * @file    comms_network.h
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


#ifndef COMMS_NETWORK_H_
#define COMMS_NETWORK_H_



/*
 * Standard Header and API Header files
 */
#include <stdint.h>
#include "network_protocol_configs.h"


/******************************************************************************/
/*                                                                            */
/*                            Macro Defines                                   */
/*                                                                            */
/******************************************************************************/

#pragma pack(1)


#define COMMS_NET_PREAMBLE_LENGTH      2
#define COMMS_NET_PAYLOAD_LENGTH       20
#define COMMS_NET_MACADDR_LENGTH       6
#define COMMS_NET_MESSAGE_BUFFER_SIZE  32


/******************************************************************************/
/*                                                                            */
/*                  Data Structures for wireless network                      */
/*                                                                            */
/******************************************************************************/


/* Wireless network SYNC Message structure Declaration */
typedef struct _sync_packet sync_packet_t;



/* network slot values */
typedef enum _network_slot
{
    net_sync_slot      = COMMS_SYNC_SLOTNUM,
    net_access_slot    = COMMS_ACCESS_SLOTNUM,
    net_broadcast_slot = COMMS_BROADCAST_SLOTNUM

}network_slot_t;




/* */
typedef struct _device_config
{
    char      device_mac[COMMS_NET_MACADDR_LENGTH];  /*!< */
    uint16_t  device_network_id;                     /*!< */
    uint8_t   network_access_slot;                   /*!< */
    uint8_t   device_slot_number;                    /*!< */
    uint16_t  device_slot_time;                      /*!< */
    uint8_t   total_slots;                           /*!< */
    uint8_t   network_joined;                        /*!< */
    uint8_t   device_count;                          /*!< */

}device_config_t;




typedef enum _message_flags
{
    CLEAR_FLAG        = 0,
    SYNC_FLAG         = 1,
    JOINREQ_FLAG      = 2,
    JOINRESP_FLAG     = 3,
    STATUSMSG_FLAG    = 4,
    CONTRLMSG_FLAG    = 6,

}message_flags_t;




typedef struct _application_flags
{
    uint8_t network_join_request      : 1;
    uint8_t network_join_response     : 1;
    uint8_t network_joined_state      : 1;
    uint8_t application_message_ready : 1;
    uint8_t network_message_ready     : 1;

}application_flags_t;



typedef struct _comms_network_buffer
{
    char receive_message[COMMS_NET_MESSAGE_BUFFER_SIZE];
    char read_message[COMMS_NET_MESSAGE_BUFFER_SIZE];

    application_flags_t application_data;

    char application_message[COMMS_NET_MESSAGE_BUFFER_SIZE];
    char network_message[COMMS_NET_MESSAGE_BUFFER_SIZE];

    message_flags_t flag_state;


}comms_network_buffer_t;



/* Network operations handle */
typedef struct _network_operations
{
    int8_t (*send_message)(char *message_buffer, uint8_t message_length);
    int8_t (*recv_message)(char *message_buffer, uint8_t message_length);
    int8_t (*set_timer)(uint16_t device_slot_time, uint8_t device_slot_number);
    int8_t (*reset_timer)(void);
    int8_t (*sync_activity_status)(void);
    int8_t (*send_activity_status)(void);
    int8_t (*recv_activity_status)(void);
    int8_t (*clear_status)(void);

}network_operations_t;



/* Network Access Control Handle */
typedef struct _access_control
{
    sync_packet_t  *sync_message;  /*!< */

    network_operations_t *network_commands;

}access_control_t;




/******************************************************************************/
/*                                                                            */
/*              Network Structure API Function Prototypes                     */
/*                                                                            */
/******************************************************************************/



/**************************************************************************************
 * @brief  Constructor function to create network access handle object
 * @param  *network_operations_t : reference to network operations handle
 * @retval access_control_t      : error: NULL, success: address of the created object
 **************************************************************************************/
access_control_t* create_network_handle(network_operations_t *network_ops);




/**************************************************************************************
 * @brief  Constructor function to create server device configure object
 * @param  *mac_address          : mac_address of the server device
 * @param  network_id            : network id of the server
 * @param  device_slot_time      : slot time interval
 * @param  total_slots           : no of existing slots at start
 * @retval device_config_t       : error: NULL, success: address of the created object
 **************************************************************************************/
device_config_t* create_server_device(char *mac_address, uint16_t network_id, uint16_t device_slot_time, uint8_t total_slots);




/*******************************************************************
 * @brief  Function to send sync message through network hardware
 * @param  *network         : reference to network handle structure
 * @param  *message_buffer  : message buffer to be send
 * @param  message_length   : message_length
 * @retval int8_t           : error: -1, success: length of message
 *******************************************************************/
int8_t comms_send(access_control_t *network, char *message_buffer, uint8_t message_length);




/*********************************************************************
 * @brief  Function to set transmission timer for slotted network
 * @param  *network  : reference to network handle structure
 * @param  *device   : reference to the device configuration structure
 * @param  slot_type : type of network slot
 * @retval int8_t    : error: -3, success: 0
 *********************************************************************/
int8_t comms_network_set_timer(access_control_t *network, device_config_t *device, network_slot_t slot_type);





/******************************************************************************/
/*                                                                            */
/*              Network Activity / Status Functions Prototypes                */
/*                                                                            */
/******************************************************************************/



/************************************************************
 * @brief  Function to enable sync activity status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -5, success = 0
 ************************************************************/
int8_t comms_sync_status(access_control_t *network);



/************************************************************
 * @brief  Function to enable send activity status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -6, success = 0
 ************************************************************/
int8_t comms_send_status(access_control_t *network);



/************************************************************
 * @brief  Function to enable receive activity status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -7, success = 0
 ************************************************************/
int8_t comms_recv_status(access_control_t *network);



/************************************************************
 * @brief  Function to enable clear activity status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -8, success = 0
 ************************************************************/
int8_t comms_clear_activity(access_control_t *network);



/*********************************************************
 * @brief  Function to calculate network message checksum
 * @param  data   : message data
 * @param  offset : starting offset for message data
 * @param  size   : length of message
 * @retval int8_t : checksum
 *********************************************************/
int8_t comms_network_checksum(char *data, uint8_t offset, uint8_t size);





/******************************************************************************/
/*                                                                            */
/*                   API Function Prototypes (client)                         */
/*                                                                            */
/******************************************************************************/




/*************************************************************************
 * @brief  Function to get sync message data
 * @param  *client_device   : reference to client device config structure
 * @param  *message_payload : message payload from syn cmessage
 * @param  network          : network handle structure
 * @retval int8_t           : error -5, success: 0
 *************************************************************************/
int8_t get_sync_data(device_config_t *client_device, char *message_payload ,access_control_t network);





/******************************************************************************/
/*                                                                            */
/*                   API Function Prototypes (Server)                         */
/*                                                                            */
/******************************************************************************/



/***********************************************************************
 * @brief  Function to configure sync message
 * @param  *server_device   : reference to server device network handle
 * @param  network          : network handle structure
 * @param  *message_payload : message payload from sync message
 * @retval int8_t           : error: 0, success: length of message
 ***********************************************************************/
uint8_t comms_network_sync_message(access_control_t *server_device, uint16_t network_id, uint16_t slot_time, char *payload);








#endif /* COMMS_NETWORK_H_ */
