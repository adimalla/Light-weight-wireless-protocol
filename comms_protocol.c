/**
 ******************************************************************************
 * @file    comms_protocol.c
 * @author  Aditya Mall,
 * @brief   (6314)comms protocol source file.
 *
 *  Info
 *          (6314)comms protocol source file, based on custom protocol,
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




/*
 * Standard Header and API Header files
 */
#include <stdint.h>
#include <stdlib.h>


#include "comms_protocol.h"
#include "network_protocol_configs.h"


/******************************************************************************/
/*                                                                            */
/*                  Data Structures and Defines                               */
/*                                                                            */
/******************************************************************************/



/* Protocol codes */
typedef enum device_return_codes
{
    DEV_FUNC_SUCCESS         =  1, /*!< */
    SYNC_DATA_ERROR          = -1, /*!< */
    CALIB_DATA_ERROR         = -2, /*!< */
    JOINREQ_OPTS_FUNC_ERROR  = -3, /*!< */
    JOINREQ_MAIN_FUNC_ERROR  = -4, /* Not Used */
    JOINRESP_FUNC_ERROR      = -5, /*!< */
    STATUSACK_FUNC_ERROR     = -6, /*!< */
    CONTRL_FUNC_ERROR        = -7, /*!< */
    JOINRESP_STATUS_ERROR    = -8, /*!< */
    STATUSMSG_RECV_ERROR     =- 9,

}device_codes_t;






/******************************************************************************/
/*                                                                            */
/*                              Private Functions                             */
/*                                                                            */
/******************************************************************************/


/********************************************************
 * @brief  static function to calculate message checksum
 * @param  data   : Message data
 * @param  offset : Starting offset for message data
 * @param  size   : Length of message
 * @retval int8_t : Error value
 ********************************************************/
int8_t static comms_checksum(char *data, uint8_t offset, uint8_t size)
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
/*                              API Functions (Client)                        */
/*                                                                            */
/******************************************************************************/



/********************************************************
 * @brief  Function to configure JOINREQ message options
 * @param  *client    : pointer to comms protocol handle
 * @param  qos        : quality of service value
 * @param  keep_alive : Keep alive request set/reset
 * @retval int8_t     : error -3, success: 1
 ********************************************************/
int8_t comms_joinreq_options(protocol_handle_t *client, uint8_t qos, uint8_t keep_alive)
{
    int8_t func_retval = 0;

    /* Handle error */
    if(qos > 1 || keep_alive > 1)
    {
        func_retval = JOINREQ_OPTS_FUNC_ERROR;
    }
    else
    {
        /* Configure flags */
        client->joinrequest_msg->join_options.quality_of_service = qos;
        client->joinrequest_msg->join_options.request_keep_alive = keep_alive;

        func_retval = DEV_FUNC_SUCCESS;
    }

    return func_retval;
}




/*******************************************************************
 * @brief  Function to configure JOINREQ message
 * @param  *client         : pointer to comms protocol handle
 * @param  device          : client device structure
 * @param  requested_slots : number of slots requested
 * @retval uint8_t         : error 0, success: length of message
 *******************************************************************/
uint8_t comms_joinreq_message(protocol_handle_t *client, device_config_t device, uint8_t requested_slots)
{

    int8_t func_retval     = 0;
    uint8_t payload_index  = 0;
    uint8_t payload_length = 0;
    uint8_t message_length = 0;
    char    payload_buff[4] = {0};

    /* Handle error */
    if(client == NULL || device.device_network_id == 0 || requested_slots > COMMS_SERVER_MAX_SLOTS)
    {

        func_retval = 0;

    }
    else
    {
        client->joinrequest_msg->preamble[0] = (PREAMBLE_JOINREQ >> 8) & 0xFF;
        client->joinrequest_msg->preamble[1] = (PREAMBLE_JOINREQ >> 0) & 0xFF;

        client->joinrequest_msg->fixed_header.message_type = COMMS_JOINREQ_MESSAGE;

        /* Put mac address */
        strncpy(client->joinrequest_msg->source_mac, device.device_mac, 6);

        /* destination mac address (not defined)*/

        /* put network id received from server */
        client->joinrequest_msg->network_id = device.device_network_id;

        client->joinrequest_msg->message_slot_number = 0;

        /* Configure slot options */
        if(requested_slots != 0)
        {
            client->joinrequest_msg->join_options.request_slot_time  = 1;
        }

        /* !! Join Options set by comms_joinreq_options functions (externally called), else options are empty !! */


        /* Add payload message */
        ltoa((long int)requested_slots, payload_buff);

        payload_length = strlen(payload_buff);

        strncpy(client->joinrequest_msg->payload, payload_buff, payload_length);


        /* Add message terminator */
        payload_index = payload_length;

        strncpy(client->joinrequest_msg->payload + payload_index, COMMS_MESSAGE_TERMINATOR, COMMS_TERMINATOR_LENGTH);


        /* Calculate remaining length */
        client->joinrequest_msg->fixed_header.message_length = COMMS_SOURCE_MACADDR_SIZE + COMMS_DESTINATION_MACADDR_SIZE + COMMS_NETWORK_ID_SIZE + COMMS_SLOTNUM_SIZE +\
                COMMS_JOIN_OPTONS_SIZE + payload_length + COMMS_TERMINATOR_LENGTH;

        /* Total message Length */
        message_length = client->joinrequest_msg->fixed_header.message_length + COMMS_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH;


        /* Calculate checksum */
        client->joinrequest_msg->fixed_header.message_checksum = comms_checksum((char*)client->joinrequest_msg, 5, message_length);

        func_retval = message_length;

    }

    return func_retval;
}



/******************************************************************************
 * @brief  Function to get JOINRESP from server
 * @param  device     : client device structure
 * @param  client     : comms protocol handle
 * @retval uint8_t    : error: -5 , success: Message status
 ******************************************************************************/
int8_t comms_get_joinresp_data(device_config_t *device, protocol_handle_t client)
{
    int8_t func_retval = 0;
    uint8_t message_status = 0;


    if(device == NULL)
    {
        func_retval = JOINRESP_FUNC_ERROR;
    }
    else
    {
        /* Initialize value of client id / device slot number */
        device->device_slot_number = 0;

        /* Check if JOINRESP message is intended for current device (MAC Address check) */
        if(strncmp(device->device_mac, client.joinresponse_msg->destination_mac, 6) == 0)
        {

            message_status = client.joinresponse_msg->fixed_header.message_status;

            /* Get client id from message payload */
            switch(message_status)
            {

            case JOINRESP_NACK:

                func_retval = client.joinresponse_msg->fixed_header.message_status;

                break;

            case JOINRESP_ACK:

                func_retval = client.joinresponse_msg->fixed_header.message_status;
                device->device_slot_number = atoi(client.joinresponse_msg->payload);

            case JOINRESP_DUP:

                func_retval = client.joinresponse_msg->fixed_header.message_status;
                device->device_slot_number  = atoi(client.joinresponse_msg->payload);

                break;

            default:

                func_retval = JOINRESP_FALSE;

                break;

            }
        }
        else
        {
            func_retval = JOINRESP_FALSE;
        }


    }

    return func_retval;
}




/****************************************************************************************
 * @brief  Function to configure STATUS message
 * @param  *client         : pointer to the protocol handle
 * @param  device          : client device structure
 * @param  destination_id  : destination id of device to send the status message to
 * @param  payload_message : Message payload to be sent to the destination device
 * @retval uint8_t         : error 0, success: length of message
 ****************************************************************************************/
uint8_t comms_status_message(protocol_handle_t *client, device_config_t device, uint8_t destination_id, const char *payload_message)
{
    uint8_t func_retval    = 0;
    uint8_t message_length = 0;
    uint8_t payload_length = 0;
    uint8_t payload_index  = 0;

    /* Calculate  message length */
    payload_length = strlen(payload_message);

    /* Wrap payload message */
    if(payload_length > COMMS_PAYLOAD_LENGTH)
    {
        payload_length = COMMS_PAYLOAD_LENGTH - 2;
    }

    /* Handle parameter error */
    if(client == NULL || payload_message == NULL || device.device_slot_number <= 3)
    {
        func_retval = 0;
    }
    else
    {
        client->status_msg->preamble[0] = (PREAMBLE_STATUS >> 8) & 0xFF;
        client->status_msg->preamble[1] = (PREAMBLE_STATUS >> 0) & 0xFF;

        client->status_msg->fixed_header.message_type = COMMS_STATUS_MESSAGE;

        client->status_msg->network_id = device.device_network_id;

        client->status_msg->message_slot_number = device.device_slot_number;

        client->status_msg->destination_client_id = destination_id;

        /* Add payload message */
        strncpy(client->status_msg->payload, payload_message, payload_length);

        /* Add Message terminator */
        payload_index = payload_length;

        strncpy(client->status_msg->payload + payload_index, COMMS_MESSAGE_TERMINATOR, COMMS_TERMINATOR_LENGTH);

        /* Calculate remaining message length */
        client->status_msg->fixed_header.message_length = COMMS_NETWORK_ID_SIZE + COMMS_SLOTNUM_SIZE + COMMS_DESTINATION_DEVICEID_SIZE + payload_length + \
                COMMS_TERMINATOR_LENGTH;

        /* Total message length */
        message_length = client->status_msg->fixed_header.message_length + COMMS_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH;

        /* Get Checksum */
        client->status_msg->fixed_header.message_checksum = comms_checksum((char*)client->status_msg, 5, message_length);

        func_retval = message_length;

    }


    return func_retval;
}




/*****************************************************
 * @brief  Function to configure STATUS message
 * @param  client : Protocol handle structure
 * @retval int8_t : error -6, success: message status
 *****************************************************/
int8_t comms_get_statusack(protocol_handle_t client, uint16_t network_id, uint8_t device_id)
{
    int8_t func_retval = 0;

    if(client.statusack_msg == NULL)
    {
        func_retval = STATUSACK_FUNC_ERROR;
    }
    else
    {
        if(client.statusack_msg->network_id == network_id && client.statusack_msg->message_slot_number == device_id)
        {
            func_retval = (int8_t)client.statusack_msg->fixed_header.message_status;
        }
    }

    return func_retval;
}





/*************************************************************************
 * @brief  Function to get CONTRL message
 * @param  message_buffer    : message data from CONTRL message
 * @param  *source_client_id : pointer to client/device id of source device
 * @param  client            : Protocol handle structure
 * @param  network_id        : network id
 * @param  device_id         : device slot number / device id
 * @retval int8_t            : error -7, success: message length of payload
 **************************************************************************/
int8_t comms_get_contrl_data(char *message_buffer, uint8_t* source_client_id, protocol_handle_t device, uint16_t network_id, uint8_t device_id)
{
    int8_t func_retval    = 0;
    int8_t message_length = 0;

    if(device.contrl_msg == NULL || network_id == 0 || device_id == 0)
    {
        func_retval = CONTRL_FUNC_ERROR;
    }
    else
    {
        /* Check if message is for receiving device and on the same network */
        if(device.contrl_msg->network_id == network_id && device.contrl_msg->destination_client_id == device_id)
        {
            /* Get source device id */
            *source_client_id = device.contrl_msg->source_client_id;

            /* Get payload data */
            message_length = (int8_t)strlen(device.contrl_msg->payload);

            strncpy(message_buffer, device.contrl_msg->payload, (size_t)(message_length - COMMS_TERMINATOR_LENGTH));

            func_retval = message_length - COMMS_TERMINATOR_LENGTH;
        }
    }

    return func_retval;
}



/******************************************************************************/
/*                                                                            */
/*                              API Functions (Sever)                         */
/*                                                                            */
/******************************************************************************/






/*******************************************************************
 * @brief  Function to set JOINRESP message status
 * @param  server       : reference to the protocol handle structure
 * @param  status_value : Join response status value
 * @retval int8_t       : error: -8, success: 0
 *******************************************************************/
int8_t comms_set_joinresp_message_status(protocol_handle_t *server, int8_t status_value)
{
    int8_t func_retval = 0;

    if(server == NULL)
    {
        func_retval = JOINRESP_STATUS_ERROR;
    }
    else
    {
        /*TODO !! if else redundant here, will be changed later */
        if(status_value == -2)
        {
            server->joinresponse_msg->fixed_header.message_status = JOINRESP_NACK;
        }
        else if(status_value == -3)
        {
            server->joinresponse_msg->fixed_header.message_status = JOINREQ_DUP;
        }
        else
        {
            server->joinresponse_msg->fixed_header.message_status = JOINRESP_ACK;
        }

        func_retval = DEV_FUNC_SUCCESS;
    }

    return func_retval;
}



/***********************************************************************************
 * @brief  Function to configure JOINRESP message
 * @param  *server         : reference to the protocol handle
 * @param  device_server   : server protocol handle structure
 * @param  destination_mac : destination mac address
 * @param  client_id       : destination client id, new id given to the client
 * @retval uint8_t         : error 0, success: length of message
 ***********************************************************************************/
uint8_t comms_joinresp_message(protocol_handle_t *server, device_config_t device_server, char *destination_mac, int8_t client_id)
{

    uint8_t func_retval = 0;
    char payload_buff[4] = {0};
    uint8_t payload_length = 0;
    uint8_t payload_index  = 0;
    uint8_t message_length = 0;


    if(server == NULL)
    {
        func_retval = 0;
    }
    else
    {
        server->joinresponse_msg->preamble[0] = (PREMABLE_JOINRESP >> 8) & 0xFF;
        server->joinresponse_msg->preamble[1] = (PREMABLE_JOINRESP >> 0) & 0xFF;


        //server->joinresponse_msg->fixed_header.message_status = JOINRESP_ACK;
        server->joinresponse_msg->fixed_header.message_type   = COMMS_JOINRESP_MESSAGE;

        memcpy(server->joinresponse_msg->source_mac,device_server.device_mac,6);
        memcpy(server->joinresponse_msg->destination_mac,destination_mac,6);

        server->joinresponse_msg->network_id = device_server.device_network_id;
        server->joinresponse_msg->message_slot_number = COMMS_SERVER_SLOTNUM;

        /* Send client id as JOINREQ payload */
        ltoa((long int)client_id,payload_buff);

        payload_length = strlen(payload_buff);

        strncpy(server->joinresponse_msg->payload, payload_buff, payload_length);

        /* Add message terminator */
        payload_index = payload_length;

        strncpy(server->joinresponse_msg->payload + payload_index, COMMS_MESSAGE_TERMINATOR, COMMS_TERMINATOR_LENGTH);

        /* Calculate remaining message length */
        server->joinresponse_msg->fixed_header.message_length = COMMS_MACADDR_SIZE + COMMS_DESTINATION_MACADDR_SIZE + COMMS_NETWORK_ID_SIZE + \
                COMMS_SLOTNUM_SIZE + payload_length + COMMS_TERMINATOR_LENGTH;

        /* Total message Length */
        message_length = server->joinresponse_msg->fixed_header.message_length + COMMS_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH;

        /* Calculate checksum */
        server->joinresponse_msg->fixed_header.message_checksum = comms_checksum((char*)server->joinresponse_msg, 5, message_length);

        func_retval = message_length;

    }

    return func_retval;
}





/****************************************************************************************
 * @brief  Function to get STATUS Message from client
 * @param  message_buffer         : message data from status message
 * @param  *source_client_id      : reference to client/device id of source device
 * @param  *destination_client_id : reference to client/device id of destination device
 * @param  device                 : reference to the client protocol handle structure
 * @retval int8_t                 : error: -9, success: length of status message payload
 ****************************************************************************************/
int8_t comms_get_status_message(char *client_payload, uint8_t *source_client_id, uint8_t *destination_client_id, protocol_handle_t device)
{
    int8_t func_retval            = 0;
    uint8_t status_payload_length = 0;


    /* 1-3 slots are reserved can't be taken by any device */
    if(device.status_msg->message_slot_number <= 3)
    {
        *destination_client_id = 0;
        func_retval = STATUSMSG_RECV_ERROR;
    }
    else
    {
        status_payload_length = strlen(device.status_msg->payload);

        /* get source client id*/
        *source_client_id = device.status_msg->message_slot_number;

        /* get destination client id */
        *destination_client_id = device.status_msg->destination_client_id;

        /* get payload data from client, get rid of the terminator */
        strncpy(client_payload, device.status_msg->payload, status_payload_length - COMMS_TERMINATOR_LENGTH);

        func_retval = status_payload_length;
    }


    return func_retval;
}






/****************************************************************************************
 * @brief  Function to configure CONTRL message
 * @param  *server                : reference to the server protocol handle
 * @param  device                 : reference to the client protocol handle structure
 * @param  *source_client_id      : reference to client/device id of source device
 * @param  *destination_client_id : reference to client/device id of destination device
 * @param  payload                : CONTRL message payload
 * @retval int8_t                 : error: 0, success: length of CONTRL message payload
 ****************************************************************************************/
uint8_t comms_control_message(protocol_handle_t *server, device_config_t device, uint8_t source_id, uint8_t destination_id, const char *payload)
{
    uint8_t func_retval    = 0;
    uint8_t payload_length = 0;
    uint8_t payload_index  = 0;
    uint8_t message_length = 0;

    payload_length = strlen(payload);

    server->contrl_msg->preamble[0] = (PREAMBLE_CONTRL >> 8) & 0xFF;
    server->contrl_msg->preamble[1] = (PREAMBLE_CONTRL >> 0) & 0xFF;


    if(destination_id == source_id)
    {
        server->contrl_msg->fixed_header.message_status = CLIENT_ECHO;
    }
    else
    {
        server->contrl_msg->fixed_header.message_status = MESSSAGE_OK;
    }


    server->contrl_msg->fixed_header.message_type = COMMS_CONTRL_MESSAGE;

    server->contrl_msg->network_id = device.device_network_id;
    server->contrl_msg->message_slot_number = device.device_slot_number;

    server->contrl_msg->source_client_id      = source_id;
    server->contrl_msg->destination_client_id = destination_id;

    /* add payload */
    strncpy(server->contrl_msg->payload, payload, payload_length);

    /* add message terminator */
    payload_index = payload_length;

    strncpy(server->contrl_msg->payload + payload_index, COMMS_MESSAGE_TERMINATOR, COMMS_TERMINATOR_LENGTH);


    /* Calculate remaining message length */
    server->contrl_msg->fixed_header.message_length = COMMS_NETWORK_ID_SIZE + COMMS_SLOTNUM_SIZE + COMMS_SOURCE_DEVICEID_SIZE + \
            COMMS_DESTINATION_DEVICEID_SIZE + payload_length + COMMS_TERMINATOR_LENGTH;

    /* calculate total message length  */
    message_length = server->contrl_msg->fixed_header.message_length + COMMS_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH;

    /* Get Checksum */
    server->contrl_msg->fixed_header.message_checksum = comms_checksum((char*)server->contrl_msg, 5, message_length);


    func_retval = message_length;


    return func_retval;
}






/*
 * client_id : client id value from table
 */
int8_t comms_statusack_message(protocol_handle_t *client, device_config_t device, int8_t client_id, uint8_t destination_client_id)
{
    uint8_t func_retval    = 0;
    uint8_t message_length = 0;


    /* Handle parameter error */
    if(client == NULL)
    {
        func_retval = 0;

    }
    else
    {
        client->statusack_msg->preamble[0] = (PREAMBLE_STATUSACK >> 8) & 0xFF;
        client->statusack_msg->preamble[1] = (PREAMBLE_STATUSACK >> 0) & 0xFF;

        if(client_id <= 0)
        {
            client->statusack_msg->fixed_header.message_status = CLIENT_NOT_FOUND;
        }
        else
        {
            client->statusack_msg->fixed_header.message_status = MESSSAGE_OK;
        }

        client->statusack_msg->fixed_header.message_type = COMMS_STATUSACK_MESSAGE;


        client->statusack_msg->network_id = device.device_network_id;
        client->statusack_msg->message_slot_number = device.device_slot_number;

        client->statusack_msg->destination_client_id = destination_client_id;

        /* Add Message terminator */
        strncpy(client->statusack_msg->payload, COMMS_MESSAGE_TERMINATOR, COMMS_TERMINATOR_LENGTH);

        /* Calculate remaining message length */
        client->statusack_msg->fixed_header.message_length = COMMS_NETWORK_ID_SIZE + COMMS_SLOTNUM_SIZE + COMMS_DESTINATION_DEVICEID_SIZE + \
                COMMS_TERMINATOR_LENGTH;

        /* Total message length */
        message_length = client->statusack_msg->fixed_header.message_length + COMMS_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH;

        /* Get Checksum */
        client->statusack_msg->fixed_header.message_checksum = comms_checksum((char*)client->statusack_msg, 5, message_length);

        func_retval = message_length;

    }


    return func_retval;
}









