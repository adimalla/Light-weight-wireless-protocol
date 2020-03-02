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
#define COMMS_NET_MESSAGE_BUFFER_SIZE  32


/******************************************************************************/
/*                                                                            */
/*                  Data Structures for wireless network                      */
/*                                                                            */
/******************************************************************************/



/* Network Device Configuration Structure for Server and Client */
typedef struct _device_config
{
    char      device_mac[NET_MAC_SIZE];  /*!< Device Mac Address, user defined                                     */
    uint16_t  device_network_id;         /*!< Device Network ID, user defined                                      */
    uint8_t   network_access_slot;       /*!< Device Access Slot number, configs header file defined               */
    uint8_t   device_slot_number;        /*!< Device Slot Number, user defined                                     */
    uint16_t  device_slot_time;          /*!< Time of each slot interval, user defined                             */
    uint8_t   total_slots;               /*!< Total number of slots, user defined, changes in server on runtime    */
    uint8_t   network_joined;            /*!< Network Joined State, changed by client on runtime                   */
    uint8_t   device_count;              /*!< Current number of device connected to the server, changes on runtime */

}device_config_t;



/* Network slot values for slotted network */
typedef enum _network_slot
{
    NET_SYNC_SLOT          = COMMS_SYNC_SLOTNUM,      /*!< Sever Sync slot number       */
    NET_ACCESS_SLOT        = COMMS_ACCESS_SLOTNUM,    /*!< Server Access Slot number    */
    NET_BROADCAST_SLOT     = COMMS_BROADCAST_SLOTNUM, /*!< Server Broadcast Slot number */
    NET_CLIENT_ACCESS_SLOT,
    NET_CLIENT_SLOT

}network_slot_t;



/* Network buffer message flags */
typedef enum _message_flags
{
    CLEAR_FLAG        = 0,  /*!< clear message flag              */
    SYNC_FLAG         = 1,  /*!< sync message get/send flag      */
    JOINREQ_FLAG      = 2,  /*!< joinreq message get/send flag   */
    JOINRESP_FLAG     = 3,  /*!< joinresp message get/send flag  */
    STATUSMSG_FLAG    = 4,  /*!< statusmsg message get/send flag */
    STATUSACK_FLAG    = 5,  /*!< statusack message get/send flag */
    CONTRLMSG_FLAG    = 6   /*!< contrlmsg message get/send flag */

}message_flags_t;


/* Application Flags bit field structure */
typedef struct _application_flags
{
    uint8_t network_join_request      : 1;  /*!< Network join request flag, user enabled               */
    uint8_t network_join_response     : 1;  /*!< Network join response flag, user enabled              */
    uint8_t network_joined_state      : 1;  /*!< Network joined state, client controlled               */
    uint8_t application_message_ready : 1;  /*!< App message ready flag, user enabled                  */
    uint8_t network_message_ready     : 1;  /*!< Network message ready flag, client/ server controlled */

}app_flags_t;



/* Network buffer structure for message passing between network and user applications */
typedef struct _comms_network_buffer
{
    app_flags_t     application_flags;                     /*!< Application flag state structure                          */
    char            receive_message[NET_DATA_LENGTH];      /*!< Receive message buffer all messages                       */
    char            read_message[NET_DATA_LENGTH];         /*!< Read message buffer for valid messages, filled by network */
    char            application_message[NET_DATA_LENGTH];  /*!< Application message buffer, filled by application         */
    char            network_message[NET_DATA_LENGTH];      /*!< Network message buffer, filled by network                 */
    uint16_t        app_message_length;                    /*!< */
    message_flags_t flag_state;                            /*!< Network message flag states                               */
    uint8_t         source_id;                             /*!< Network message source ID                                 */
    uint8_t         destination_id;                        /*!< Network Message destination ID                            */

}comms_network_buffer_t;



/* Network operations callback structure */
typedef struct _network_operations
{
    /* Send/Receive function operations */
    int8_t (*send_message)(char *message_buffer, uint16_t message_length);          /*!< Send function               */
    int8_t (*recv_message)(char *message_buffer, uint16_t message_length);          /*!< Receive function            */

    /* Transmit timer and receive interrupt operations */
    int8_t (*set_tx_timer)(uint16_t device_slot_time, uint8_t device_slot_number);  /*!< Set transmit timer function */
    int8_t (*reset_tx_timer)(void);                                                 /*!< Reset transmit timer        */
    int8_t (*clear_recv_interrupt)(void);                                           /*!< Clear receive interrupt     */

    /* Timeout operations */
    int8_t (*request_timeout)(uint8_t timeout_seconds);                             /*!< Request timeout             */
    int8_t (*response_timeout)(uint8_t timeout_seconds);                            /*!< Response timeout            */

#if ACTIVITY_OPERATIONS
    /* Network activity status operations */
    int8_t (*sync_activity_status)(void);                                           /*!< Sync status                 */
    int8_t (*send_activity_status)(void);                                           /*!< Send status                 */
    int8_t (*recv_activity_status)(void);                                           /*!< Read status                 */
    int8_t (*clear_status)(void);                                                   /*!< Clear status                */
    int8_t (*net_connected_status)(void);                                           /*!< Network connected status    */
#endif

#if DEBUG_OPERATIONS
    /* Network debug operations */
    int8_t (*net_debug_print)(char *debug_message);                                /*!< Network debug print          */

#endif

}network_operations_t;


/* Network header */
typedef struct _wi_network_header
{
    uint8_t message_status    : 4;  /*!< (LSB) Message Status  */
    uint8_t message_type      : 4;  /*!< (MSB) Type of Message */
    uint8_t message_length;         /*!< Length of message     */
    uint8_t message_checksum;       /*!< Message Checksum      */

}net_header_t;


/* Network message structure */
struct _network_message
{
    char         preamble[NET_PREAMBLE_LENTH];  /*!< Message preamble */
    net_header_t fixed_header;                  /*!< Network Header   */

};

/* Network Header structure declaration */
typedef struct _network_message network_message_t;


/* SYNC Message structure declaration */
typedef struct _sync_packet sync_packet_t;


/* Network Access Control Handle */
typedef struct _access_control
{
    network_message_t    *packet_type;       /*!< Network message packet structure */
    sync_packet_t        *sync_message;      /*!< Sync message packet structure    */
    network_operations_t *network_commands;  /*!< Network operations structure     */

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
device_config_t* create_server_device(char *mac_address, uint16_t network_id, uint16_t device_slot_time,
                                      uint8_t total_slots);


/**************************************************************************************
 * @brief  Constructor function to create client device configure object
 * @param  *mac_address          : mac_address of the server device
 * @param  requested_total_slots : number of slots requested
 * @retval device_config_t       : error: NULL, success: address of the created object
 **************************************************************************************/
device_config_t* create_client_device(char *mac_address, uint8_t requested_total_slots);


/*******************************************************************
 * @brief  Function to send sync message through network hardware
 * @param  *network         : reference to network handle structure
 * @param  *message_buffer  : message buffer to be send
 * @param  message_length   : message_length
 * @retval int8_t           : error: -1, success: length of message
 *******************************************************************/
int8_t comms_send(access_control_t *network, char *message_buffer, uint16_t message_length);


/************************************************************************
 * @brief  Function to receive message through network hardware interrupt
 * @param  *network         : reference to network handle structure
 * @param  *message_buffer  : message buffer to be send
 * @param  *read_index      : index of buffer loop
 * @retval int8_t           : error: -2, success: length of message
 ************************************************************************/
int8_t comms_server_recv_it(access_control_t *network, comms_network_buffer_t *recv_buffer, uint8_t *read_index);


/************************************************************************
 * @brief  Function to receive message through network hardware interrupt
 * @param  *network         : reference to network handle structure
 * @param  *message_buffer  : message buffer to be send
 * @param  *read_index      : index of buffer loop
 * @retval int8_t           : error: -2, success: length of message
 ************************************************************************/
int8_t comms_client_recv_it(access_control_t *network, comms_network_buffer_t *recv_buffer, uint8_t *read_index);


/*******************************************************************
 * @brief  Function to send application message.
 * @param  *network         : reference to network buffer structure
 * @param  *message_buffer  : user message
 * @param  message_length   : message length
 * @retval int8_t           : error: -2, success: length of message
 *******************************************************************/
int8_t send_application_message(comms_network_buffer_t *network_buffer, char *user_message, uint16_t message_length);


/*********************************************************************
 * @brief  Function to set transmission timer for slotted network
 * @param  *network  : reference to network handle structure
 * @param  *device   : reference to the device configuration structure
 * @param  slot_type : type of network slot
 * @retval int8_t    : error: -3, success: 0
 *********************************************************************/
int8_t comms_network_set_timer(access_control_t *network, device_config_t *device, network_slot_t slot_type);


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
/*              Network Activity / Status Functions Prototypes                */
/*                                                                            */
/******************************************************************************/



/************************************************************
 * @brief  Function to enable sync activity status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -8, success = 0
 ************************************************************/
int8_t comms_sync_status(access_control_t *network);



/************************************************************
 * @brief  Function to enable send activity status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -9, success = 0
 ************************************************************/
int8_t comms_send_status(access_control_t *network);



/************************************************************
 * @brief  Function to enable receive activity status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -10, success = 0
 ************************************************************/
int8_t comms_recv_status(access_control_t *network);



/************************************************************
 * @brief  Function to enable clear activity status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -11, success = 0
 ************************************************************/
int8_t comms_clear_activity(access_control_t *network);




/************************************************************
 * @brief  Function to enable network connected status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -13, success = 0
 ************************************************************/
int8_t comms_net_connected_status(access_control_t *network);




/******************************************************************************/
/*                                                                            */
/*                     Network Debug prototypes                               */
/*                                                                            */
/******************************************************************************/



/**********************************************************************
 * @brief  Function to print joinreq debug message
 *         Prints: " JOINREQ (SL:x) "
 * @param  *network             : reference to network handle structure
 * @param  debug_message        : debug message
 * @param  requested slots (SL) : requested number of slots
 * @retval int8_t               : error = -14, success = 0
 **********************************************************************/
int8_t comms_joinreq_debug_print(access_control_t *network, char *debug_message, uint8_t requested_slots);



/************************************************************************
 * @brief  Function to print joinresp debug message
 *         Prints: " JOINRESP (ID:x) "
 * @param  *network              : reference to network handle structure
 * @param  debug_message         : debug message
 * @param  received_slot_id (ID) : received slot ID
 * @retval int8_t                : error = -15, success = 0
 ************************************************************************/
int8_t comms_joinresp_debug_print(access_control_t *network, char *debug_message, uint8_t received_slots_id);




/***********************************************************************
 * @brief  Function to print status message debug
 *         Prints: " STATUS (DID:x LEN:x) DATA: 'message' "
 * @param  *network             : reference to network handle structure
 * @param  debug_message        : debug message
 * @param  destination_id (DID) : destination device ID
 * @param  payload_data         : payload message
 * @retval int8_t               : error = -16, success = 0
 ***********************************************************************/
int8_t comms_status_debug_print(access_control_t *network, char *debug_message, uint8_t destination_id,
                                char *payload_data);




/*******************************************************************
 * @brief  Function to print status message debug
 *         Prints: " CONTRL (SID:x LEN:x) DATA: 'message' "
 * @param  *network        : reference to network handle structure
 * @param  debug_message   : debug message
 * @param  source_id (DID) : source device ID
 * @param  payload_data    : payload message
 * @retval int8_t          : error = -16, success = 0
 *******************************************************************/
int8_t comms_contrl_debug_print(access_control_t *network, char *debug_message, uint8_t source_id,
                                char *payload_data);



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
 * @retval int8_t           : error -12, success: 0
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
 * @param  payload_size     : payload size
 * @retval int8_t           : error: 0, success: length of message
 ***********************************************************************/
uint8_t comms_network_sync_message(access_control_t *server_device, uint16_t network_id, uint16_t slot_time,
                                   char *payload, uint16_t payload_size);








#endif /* COMMS_NETWORK_H_ */
