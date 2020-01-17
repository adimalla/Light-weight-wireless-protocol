/*
 ******************************************************************************
 * @file    comms_protocol.h
 * @author  Aditya Mall,
 * @brief   (6314)comms protocol header file.
 *
 *  Info
 *          (6314)comms protocol header file, based on custom protocol,
 *          part of EE6314 IOT project (University of Texas, Arlington)
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


#ifndef __COMMS_PROTOCOL_H
#define __COMMS_PROTOCOL_H




/*
 * Standard Header and API Header files
 */
#include <string.h>
#include <stdint.h>

#include "comms_network.h"


/******************************************************************************/
/*                                                                            */
/*                            Macro Defines                                   */
/*                                                                            */
/******************************************************************************/


#pragma pack(1)


#define PREAMBLE_LENGTH 2
#define PAYLOAD_LENGTH  20



/******************************************************************************/
/*                                                                            */
/*                  Data Structures for comms protocol Messages               */
/*                                                                            */
/******************************************************************************/



typedef enum comms_message_status_codes
{
    JOINRESP_ACK      = 0,  /*!< */
    JOINRESP_NACK     = 1,  /*!< */
    JOINREQ_DUP       = 2,  /*!< */
    JOINRESP_DUP      = 2,  /*!< */
    CLIENT_ECHO       = 3,  /*!< */
    CLIENT_NOT_FOUND  = 4,  /*!< */
    MESSSAGE_OK       = 5,  /*!< */
    JOINRESP_FALSE    = 6,  /*!< */

}comms_message_status;




/* Wireless Network Protocol Messages*/


typedef struct comms_header
{
    uint8_t message_status    : 4;  /*!< (LSB)*/
    uint8_t message_type      : 4;  /*!< (MSB)*/
    uint8_t message_length;         /*!< */
    uint8_t message_checksum;       /*!< */

}comms_header_t;




typedef struct join_opts
{
    uint8_t reserved           : 4; /*!< (LSB) */
    uint8_t request_slot_time  : 1; /*!< (MSB) */
    uint8_t request_keep_alive : 1; /*!< (MSB) */
    uint8_t quality_of_service : 2; /*!< (MSB) */

}join_opts_t;



typedef struct joinreq
{
    char           preamble[PREAMBLE_LENGTH];  /*!< */
    comms_header_t fixed_header;               /*!< */
    char           source_mac[6];              /*!< */
    char           destination_mac[6];         /*!< */
    uint16_t       network_id;                 /*!< */
    uint8_t        message_slot_number;        /*!< */
    join_opts_t    join_options;               /*!< */
    char           payload[PAYLOAD_LENGTH];    /*!< */

}joinreq_t;



typedef struct joinresp
{
    char           preamble[PREAMBLE_LENGTH];  /*!< */
    comms_header_t fixed_header;               /*!< */
    char           source_mac[6];              /*!< */
    char           destination_mac[6];         /*!< */
    uint16_t       network_id;                 /*!< */
    uint8_t        message_slot_number;        /*!< */
    char           payload[PAYLOAD_LENGTH];    /*!< */

}joinresp_t;



typedef struct status
{
    char           preamble[PREAMBLE_LENGTH];  /*!< */
    comms_header_t fixed_header;               /*!< */
    uint16_t       network_id;                 /*!< */
    uint8_t        message_slot_number;        /*!< */
    uint8_t        destination_client_id;      /*!< */
    char           payload[PAYLOAD_LENGTH];    /*!< */

}status_t;




typedef struct statusack
{
    char           preamble[PREAMBLE_LENGTH]; /*!< */
    comms_header_t fixed_header;              /*!< */
    uint16_t       network_id;                /*!< */
    uint8_t        message_slot_number;       /*!< */
    uint8_t        destination_client_id;     /*!< */
    char           payload[PAYLOAD_LENGTH];   /*!< */

}statusack_t;




typedef struct contrl
{
    char           preamble[PREAMBLE_LENGTH]; /*!< */
    comms_header_t fixed_header;              /*!< */
    uint16_t       network_id;                /*!< */
    uint8_t        message_slot_number;       /*!< */
    uint8_t        source_client_id;          /*!< */
    uint8_t        destination_client_id;     /*!< */
    char           payload[PAYLOAD_LENGTH];   /*!< */

}contrl_t;





typedef struct _network_message
{
    char preamble[PREAMBLE_LENGTH];  /*!< */
    comms_header_t fixed_header;     /*!< */

}network_message_t;



typedef struct wi_net_protocol_handle
{
    network_message_t *packet_type;       /*!< */
    joinreq_t         *joinrequest_msg;   /*!< */
    joinresp_t        *joinresponse_msg;  /*!< */
    status_t          *status_msg;        /*!< */
    statusack_t       *statusack_msg;     /*!< */
    contrl_t          *contrl_msg;        /*!< */

}protocol_handle_t;






/******************************************************************************/
/*                                                                            */
/*                    API Function Prototypes (Client)                        */
/*                                                                            */
/******************************************************************************/



/********************************************************
 * @brief  Function to configure JOINREQ message options
 * @param  *device    : pointer to comms protocol handle
 * @param  qos        : quality of service value
 * @param  keep_alive : Keep alive request set/reset
 * @retval int8_t     : error -3, success: 1
 ********************************************************/
int8_t comms_joinreq_options(protocol_handle_t *device, uint8_t qos, uint8_t keep_alive);




/****************************************************************
 * @brief  Function to configure JOINREQ message
 * @param  *client         : pointer to comms protocol handle
 * @param  device          : client device structure
 * @param  requested_slots : number of slots requested
 * @retval uint8_t         : error 0, success: length of message
 ****************************************************************/
uint8_t comms_joinreq_message(protocol_handle_t *client, device_config_t device, uint8_t requested_slots);




/************************************************************
 * @brief  Function to get JOINRESP from server
 * @param  device     : client device structure
 * @param  client     : comms protocol handle
 * @retval uint8_t    : error: -5 , success: Message status
 ***********************************************************/
int8_t comms_get_joinresp_data(device_config_t *device, protocol_handle_t client);




/************************************************************************************
 * @brief  Function to configure STATUS message
 * @param  *client         : pointer to the protocol handle
 * @param  device          : client device structure
 * @param  destination_id  : destination id of device to send the status message to
 * @param  payload_message : Message payload to be sent to the destination device
 * @retval uint8_t         : error 0, success: length of message
 ***********************************************************************s*************/
uint8_t comms_status_message(protocol_handle_t *client, device_config_t device, uint8_t destination_id, const char *payload_message);




/*****************************************************
 * @brief  Function to configure STATUS message
 * @param  client : Protocol handle structure
 * @retval int8_t : error -6, success: message status
 *****************************************************/
int8_t comms_get_statusack(protocol_handle_t client, uint16_t network_id, uint8_t device_id);




/*************************************************************************
 * @brief  Function to get CONTRL message
 * @param  message_buffer    : message data from CONTRL message
 * @param  *source_client_id : pointer to client/device id of source device
 * @param  client            : Protocol handle structure
 * @param  network_id        : network id
 * @param  device_id         : device slot number / device id
 * @retval int8_t            : error -7, success: message length of payload
 **************************************************************************/
int8_t comms_get_contrl_data(char *message_buffer, uint8_t *source_client_id, protocol_handle_t device, uint16_t network_id, uint8_t device_id);





/******************************************************************************/
/*                                                                            */
/*                    API Function Prototypes (Server)                        */
/*                                                                            */
/******************************************************************************/




/*******************************************************************
 * @brief  Function to set JOINRESP message status
 * @param  server       : reference to the protocol handle structure
 * @param  status_value : Join response status value
 * @retval int8_t       : error: -8, success: 0
 *******************************************************************/
int8_t comms_set_joinresp_message_status(protocol_handle_t *server, int8_t status_value);




/***********************************************************************************
 * @brief  Function to configure JOINRESP message
 * @param  *server         : reference to the protocol handle
 * @param  device_server   : server device structure
 * @param  destination_mac : destination mac address
 * @param  client_id       : destination client id, new id given to the client
 * @retval uint8_t         : error 0, success: length of message
 ***********************************************************************s***********/
uint8_t comms_joinresp_message(protocol_handle_t *server, device_config_t device_server, char *destination_mac, int8_t client_id);




/****************************************************************************************
 * @brief  Function to get STATUS Message from client
 * @param  message_buffer         : message data from status message
 * @param  *source_client_id      : pointer to client/device id of source device
 * @param  *destination_client_id : pointer to client/device id of destination device
 * @param  device                 : reference to the client protocol handle structure
 * @retval int8_t                 : error: -9, success: length of status message payload
 ****************************************************************************************/
int8_t comms_get_status_message(char *client_payload, uint8_t *source_client_id, uint8_t *destination_client_id, protocol_handle_t client);




/****************************************************************************************
 * @brief  Function to configure CONTRL message
 * @param  *server                : reference to the server protocol handle
 * @param  device                 : reference to the client protocol handle structure
 * @param  *source_client_id      : reference to client/device id of source device
 * @param  *destination_client_id : reference to client/device id of destination device
 * @param  payload                : CONTRL message payload
 * @retval int8_t                 : error: 0, success: length of CONTRL message payload
 ****************************************************************************************/
uint8_t comms_control_message(protocol_handle_t *server, device_config_t device, uint8_t source_id, uint8_t destination_id, const char *payload);




/****************************************************************************
 * @brief  Function to check/get JOINREQ message data
 * @param  server              : reference to the protocol handle structure
 * @param  server_device       : reference to the device structure
 * #param  join_response_state : state of join response flag
 * @retval int8_t              : error: -10, success: 4
 ****************************************************************************/
int8_t comms_get_joinreq_data(protocol_handle_t server, device_config_t server_device, int8_t joinresponse_state);




int8_t comms_statusack_message(protocol_handle_t *client, device_config_t device, int8_t client_id, uint8_t destination_client_id);





#endif

