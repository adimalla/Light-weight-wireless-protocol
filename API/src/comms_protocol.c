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


#include <WI_API/inc/comms_protocol.h>
#include <WI_API/inc/network_protocol_configs.h>




/******************************************************************************/
/*                                                                            */
/*                  Data Structures and Defines                               */
/*                                                                            */
/******************************************************************************/


/* JOINREQ options */
typedef struct _join_options
{
    uint8_t reserved           : 4; /*!< (LSB) Reserved                                                */
    uint8_t request_slots      : 1; /*!< (MSB) Request slots from server                               */
    uint8_t request_keep_alive : 1; /*!< (MSB) Request keep alive at server                            */
    uint8_t quality_of_service : 2; /*!< (MSB) Quality of service, Fire and Forget: 0, Atleast Once: 1 */

}join_opts_t;


/* Slot request and network name and password */
typedef struct _join_username_password
{
    uint8_t  slots_requested;  /*!< Number of slots requested */
    char     user_name[10];    /*!< Network user name         */
    uint8_t  password[10];     /*!< Network Password          */

}join_user_pswd_t;


/* JOINREQ message structure */
struct _joinreq
{
    char           preamble[NET_PREAMBLE_LENTH];   /*!< Message preamble           */
    comms_header_t fixed_header;                   /*!< Network header             */
    char           source_mac[NET_MAC_SIZE];       /*!< Source mac address         */
    char           destination_mac[NET_MAC_SIZE];  /*!< Destination mac address    */
    uint16_t       network_id;                     /*!< Network ID                 */
    uint8_t        message_slot_number;            /*!< Device/Message slot number */ /*Can be used as requested id */
    join_opts_t    join_options;                   /*!< JOINREQ message options    */
    uint8_t        payload;                        /*!< JOINREQ message payload    */

};



/* JOINRESP message structure */
struct _joinresp
{
    char           preamble[NET_PREAMBLE_LENTH];   /*!< Message PREAMBLE           */
    comms_header_t fixed_header;                   /*!< Network header             */
    char           source_mac[NET_MAC_SIZE];       /*!< Source MAC address         */
    char           destination_mac[NET_MAC_SIZE];  /*!< Destination MAC address    */
    uint16_t       network_id;                     /*!< Network ID                 */
    uint8_t        message_slot_number;            /*!< Device/Message slot number */
    uint8_t        payload;                        /*!< JOINRESP message payload   */

};



/* STATUS message structure */
struct _status
{
    char           preamble[NET_PREAMBLE_LENTH];  /*!< Message preamble           */
    comms_header_t fixed_header;                  /*!< Network header             */
    uint16_t       network_id;                    /*!< Network ID                 */
    uint8_t        message_slot_number;           /*!< Device/Message slot number */
    uint8_t        destination_client_id;         /*!< Destination Client ID      */
    uint8_t        payload;                       /*!< status message payload     */

};



/* CONTRL message structure */
struct _contrl
{
    char           preamble[NET_PREAMBLE_LENTH];  /*!< Message preamble           */
    comms_header_t fixed_header;                  /*!< Network header             */
    uint16_t       network_id;                    /*!< Network ID                 */
    uint8_t        message_slot_number;           /*!< Device/Message slot number */
    uint8_t        source_client_id;              /*!< Source Client ID           */
    uint8_t        destination_client_id;         /*!< Destination Client ID      */
    uint8_t        payload;                       /*!< CONTRL message payload     */

};



/* Not tested */
struct _statusack
{
    char           preamble[NET_PREAMBLE_LENTH]; /*!< */
    comms_header_t fixed_header;                 /*!< */
    uint16_t       network_id;                   /*!< */
    uint8_t        message_slot_number;          /*!< */
    uint8_t        destination_client_id;        /*!< */
    char           payload[PAYLOAD_LENGTH];      /*!< */

};





/* Protocol codes */
typedef enum _protocol_api_retval
{
    DEV_FUNC_SUCCESS         =  1, /*!< */
    JOINREQ_OPTS_FUNC_ERROR  = -3, /*!< */
    JOINREQ_MAIN_FUNC_ERROR  = -4, /* Not Used */
    JOINRESP_FUNC_ERROR      = -5, /*!< */
    STATUSACK_FUNC_ERROR     = -6, /*!< */
    CONTRL_FUNC_ERROR        = -7, /*!< */
    JOINRESP_STATUS_ERROR    = -8, /*!< */
    STATUSMSG_RECV_ERROR     = -9, /*!< */

}protocol_api_t;





/******************************************************************************/
/*                                                                            */
/*                              Private Functions                             */
/*                                                                            */
/******************************************************************************/


/********************************************************
 * @brief  static function to calculate message checksum
 * @param  data   : message data
 * @param  offset : starting offset for message data
 * @param  size   : length of message
 * @retval int8_t : checksum
 ********************************************************/
static int8_t comms_checksum(char *data, uint8_t offset, uint8_t size)
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
    uint8_t payload_length = 0;
    uint8_t message_length = 0;

    join_user_pswd_t *join_options_2;

    char *copy_payload;

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
        strncpy(client->joinrequest_msg->source_mac, device.device_mac, NET_MAC_SIZE);

        /* destination mac address (not defined)*/

        /* put network id received from server */
        client->joinrequest_msg->network_id = device.device_network_id;

        client->joinrequest_msg->message_slot_number = 0;

        /* Configure slot options */
        if(requested_slots != 0)
        {
            client->joinrequest_msg->join_options.request_slots = 1;
        }

        /* !! Join Options set by comms_joinreq_options functions (externally called), else options are empty !! */


        /* Add payload message */

        join_options_2 = (void*)&client->joinrequest_msg->payload;

        join_options_2->slots_requested = requested_slots;

        strcpy(join_options_2->user_name, device.user_name);

        memcpy(join_options_2->password, device.password, 10);

        /* Join option 2 length, slot(1) + name(10) + password(10) = 21 */
        payload_length = 21;

        /* Null terminator */
        copy_payload = (void*)((uint8_t*)&client->joinrequest_msg->payload + 21);

        strncpy(copy_payload, COMMS_MESSAGE_TERMINATOR, COMMS_TERMINATOR_LENGTH);

        /* Calculate remaining length */
        client->joinrequest_msg->fixed_header.message_length = JOINREQ_HEADER_SIZE + payload_length + COMMS_TERMINATOR_LENGTH;

        /* Total message Length */
        message_length = client->joinrequest_msg->fixed_header.message_length + NET_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH;

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

    char *joinresp_data;

    if(device == NULL)
    {
        func_retval = JOINRESP_FUNC_ERROR;
    }
    else
    {
        /* Get JOINRESP payload data */
        joinresp_data = (void*)&client.joinresponse_msg->payload;

        /* Initialize value of client id / device slot number */
        device->device_slot_number = 0;

        /* Check if JOINRESP message is intended for current device (MAC Address check) */
        if(strncmp(device->device_mac, client.joinresponse_msg->destination_mac, NET_MAC_SIZE) == 0)
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
                device->device_slot_number = atoi(joinresp_data);

            case JOINRESP_DUP:

                func_retval = client.joinresponse_msg->fixed_header.message_status;
                device->device_slot_number  = atoi(joinresp_data);

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
 *
 * @retval uint8_t         : error 0, success: length of message
 ****************************************************************************************/
uint8_t comms_status_message(protocol_handle_t *client, device_config_t device, uint8_t destination_id,
                             const char *payload_message, uint16_t payload_length)
{
    uint8_t func_retval    = 0;
    uint8_t message_length = 0;
    uint8_t payload_index  = 0;

    char *copy_payload;

    /* Truncate PAYLOAD message */
    if(payload_length > NET_DATA_LENGTH - COMMS_TERMINATOR_LENGTH)
    {
        payload_length = NET_DATA_LENGTH - 2;
    }

    /* Handle parameter error */
    if(client == NULL || device.device_slot_number <= 3)
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
        copy_payload = (void*)&client->status_msg->payload;

        memcpy(copy_payload, payload_message, payload_length);

        /* Add Message terminator */
        payload_index = payload_length;

        strncpy(copy_payload + payload_index, COMMS_MESSAGE_TERMINATOR, COMMS_TERMINATOR_LENGTH);

        /* Calculate remaining message length */
        client->status_msg->fixed_header.message_length = STATUS_HEADER_SIZE + payload_length + COMMS_TERMINATOR_LENGTH;

        /* Total message length */
        message_length = client->status_msg->fixed_header.message_length + NET_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH;

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
int8_t comms_get_contrl_data(char *message_buffer, uint8_t* source_client_id, protocol_handle_t device,
                             uint16_t network_id, uint8_t device_id)
{
    int8_t func_retval    = 0;
    int8_t message_length = 0;

    char *contrl_data;

    if(device.contrl_msg == NULL || network_id == 0 || device_id == 0)
    {
        func_retval = CONTRL_FUNC_ERROR;
    }
    else
    {
        /* Get CONTRL message data */
        contrl_data = (void*)&device.contrl_msg->payload;

        /* Check if message is for receiving device and on the same network */
        if(device.contrl_msg->network_id == network_id && device.contrl_msg->destination_client_id == device_id)
        {
            /* Get source device id */
            *source_client_id = device.contrl_msg->source_client_id;

            /* Get payload data */
            message_length = (int8_t)strlen(contrl_data);

            memcpy(message_buffer, contrl_data, (size_t)(message_length - COMMS_TERMINATOR_LENGTH));

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

    uint8_t func_retval    = 0;
    uint8_t payload_length = 0;
    uint8_t payload_index  = 0;
    uint8_t message_length = 0;

    char *copy_payload;
    char payload_buff[4] = {0};

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

        memcpy(server->joinresponse_msg->source_mac, device_server.device_mac, NET_MAC_SIZE);
        memcpy(server->joinresponse_msg->destination_mac, destination_mac, NET_MAC_SIZE);

        server->joinresponse_msg->network_id = device_server.device_network_id;
        server->joinresponse_msg->message_slot_number = COMMS_SERVER_SLOTNUM;

        /* Send client id as JOINREQ payload */

        copy_payload = (void*)&server->joinresponse_msg->payload;

        ltoa((long int)client_id, payload_buff);

        payload_length = strlen(payload_buff);

        memcpy(copy_payload, payload_buff, payload_length);

        /* Add message terminator */
        payload_index = payload_length;

        strncpy(copy_payload + payload_index, COMMS_MESSAGE_TERMINATOR, COMMS_TERMINATOR_LENGTH);

        /* Calculate remaining message length */
        server->joinresponse_msg->fixed_header.message_length = JOINRESP_HEADER_SIZE + payload_length + COMMS_TERMINATOR_LENGTH;

        /* Total message Length */
        message_length = server->joinresponse_msg->fixed_header.message_length + NET_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH;

        /* Calculate checksum */
        server->joinresponse_msg->fixed_header.message_checksum = comms_checksum((char*)server->joinresponse_msg, 5, message_length);

        func_retval = message_length;

    }

    return func_retval;
}


/******************************************************************************************
 * @brief  Function to get STATUS Message from client
 * @param  server                 : reference to the server protocol handle structure
 * @param  server_device          : reference to the server device configuration structure
 * @param  message_buffer         : message data from status message
 * @param  *source_client_id      : pointer to client/device id of source device
 * @param  *destination_client_id : pointer to client/device id of destination device
 * @retval int16_t                 : error: -9, success: length of status message payload
 ******************************************************************************************/
int16_t comms_get_status_message(protocol_handle_t server, device_config_t server_device, char *client_payload,
                                 uint8_t *source_client_id, uint8_t *destination_client_id)
{
    int16_t func_retval           = 0;
    int16_t status_payload_length = 0;

    char *status_data;

    /* 1-3 slots are reserved can't be taken by any device */
    if(server.status_msg->message_slot_number <= 3)
    {
        *destination_client_id = 0;
        func_retval = STATUSMSG_RECV_ERROR;
    }
    else
    {
        /* Get Status Data */
        status_data = (void*)&server.status_msg->payload;

        /* Check network ID */
        if(server.status_msg->network_id == server_device.device_network_id)
        {
            status_payload_length = abs(server.status_msg->fixed_header.message_length - \
                                        (STATUS_HEADER_SIZE + COMMS_TERMINATOR_LENGTH));

            /* get source client id*/
            *source_client_id = server.status_msg->message_slot_number;

            /* get destination client id */
            *destination_client_id = server.status_msg->destination_client_id;

            /* get payload data from client, get rid of the terminator */
            memcpy(client_payload, status_data, status_payload_length);

            func_retval = status_payload_length;
        }
    }


    return func_retval;
}


/****************************************************************************************
 * @brief  Function to configure CONTRL message
 * @param  *server                : reference to the server protocol handle
 * @param  device                 : reference to the server device structure
 * @param  *source_client_id      : reference to client/device id of source device
 * @param  *destination_client_id : reference to client/device id of destination device
 * @param  *payload               : CONTRL message payload
 * @param  payload_length         : CONTRL message payload length
 * @retval int8_t                 : error: 0, success: length of CONTRL message payload
 ****************************************************************************************/
uint8_t comms_control_message(protocol_handle_t *server, device_config_t device, uint8_t source_id,
                              uint8_t destination_id, const char *payload, uint16_t payload_length)
{
    uint8_t func_retval      = 0;
    uint8_t payload_index    = 0;
    uint8_t message_length   = 0;
    uint8_t payload_length_2 = 0;

    char *copy_payload;

    server->contrl_msg->preamble[0] = (PREAMBLE_CONTRL >> 8) & 0xFF;
    server->contrl_msg->preamble[1] = (PREAMBLE_CONTRL >> 0) & 0xFF;

    server->contrl_msg->fixed_header.message_type = COMMS_CONTRL_MESSAGE;

    server->contrl_msg->network_id            = device.device_network_id;
    server->contrl_msg->message_slot_number   = device.device_slot_number;
    server->contrl_msg->source_client_id      = source_id;
    server->contrl_msg->destination_client_id = destination_id;


    /* Get payload */
    copy_payload = (void*)&server->contrl_msg->payload;

    /* Client echo condition */
    if(destination_id == source_id)
    {
        server->contrl_msg->fixed_header.message_status = CLIENT_ECHO;

        server->contrl_msg->source_client_id = device.device_slot_number;

        /* add payload */
        memcpy(copy_payload, payload, payload_length);
    }
    /* Client not found condition */
    else if(destination_id == 0)
    {
        server->contrl_msg->fixed_header.message_status = CLIENT_NOT_FOUND;
        server->contrl_msg->source_client_id            = device.device_slot_number;
        server->contrl_msg->destination_client_id       = source_id;

        /* add NOT FOUND condition to payload */
        payload_length = 16;

        memcpy(copy_payload, "DEVICE NOT FOUND", payload_length);
    }
    else if(destination_id == 1)
    {
        server->contrl_msg->fixed_header.message_status = CLIENT_ECHO;
        server->contrl_msg->source_client_id            = device.device_slot_number;
        server->contrl_msg->destination_client_id       = source_id;

        /* Add ECHO condition to payload */
        payload_length_2 = 7;

        memcpy(copy_payload, "[Echo]:", payload_length_2);

        payload_index = payload_length_2;

        /* Add payload */
        memcpy(copy_payload + payload_index, payload, payload_length);

        payload_length += payload_length_2;
    }
    else
    {
        server->contrl_msg->fixed_header.message_status = MESSSAGE_OK;

        /* add payload */
        memcpy(copy_payload, payload, payload_length);
    }


    /* add message terminator */
    payload_index = payload_length;

    strncpy(copy_payload + payload_index, COMMS_MESSAGE_TERMINATOR, COMMS_TERMINATOR_LENGTH);

    /* Calculate remaining message length */
    server->contrl_msg->fixed_header.message_length = CONTRL_HEADER_SIZE + payload_length + COMMS_TERMINATOR_LENGTH;

    /* calculate total message length  */
    message_length = server->contrl_msg->fixed_header.message_length + NET_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH;

    /* Get Checksum */
    server->contrl_msg->fixed_header.message_checksum = comms_checksum((char*)server->contrl_msg, 5, message_length);

    func_retval = message_length;

    return func_retval;
}




/*****************************************************************************
 * @brief  Function to check/get JOINREQ message data
 * @param *client_mac_address     : client mac address
 * @param *client_requested_slots : requested slots by the client
 * @param  server                 : reference to the protocol handle structure
 * @param  server_device          : reference to the device structure
 * #param  join_response_state    : state of join response flag
 * @retval int8_t                 : error: -10, success: 4
 *****************************************************************************/
int8_t comms_get_joinreq_data(char *client_mac_address, uint8_t *client_requested_slots, protocol_handle_t server,
                              device_config_t server_device, int8_t joinresponse_state)
{
    int8_t  func_retval = 0;

    join_user_pswd_t *join_options_2;

    if(server.joinrequest_msg == NULL)
    {
        func_retval = -10;
    }
    else
    {

        /* Get JOINREQ data */
        join_options_2 = (void*)&server.joinrequest_msg->payload;

        /* Check network id */
        if( (server.joinrequest_msg->network_id == server_device.device_network_id) && (joinresponse_state == 1) )
        {

            /* Authentication check */
            if( (strncmp(server_device.user_name, join_options_2->user_name, 10) == 0 ) && \
                    (memcmp(server_device.password, join_options_2->password, 10) == 0 ) )
            {
                /* Get client requested slots */
                *client_requested_slots = join_options_2->slots_requested;

                /* Get client MAC address */
                memcpy(client_mac_address, server.joinrequest_msg->source_mac, NET_MAC_SIZE);

                /* Can be used for as JOINRESP fsm state value */
                func_retval = 4;
            }

        }

    }

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

        if(client_id == 0)
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
        client->statusack_msg->fixed_header.message_length = STATUSACK_HEADER_SIZE + COMMS_TERMINATOR_LENGTH;

        /* Total message length */
        message_length = client->statusack_msg->fixed_header.message_length + NET_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH;

        /* Get Checksum */
        client->statusack_msg->fixed_header.message_checksum = comms_checksum((char*)client->statusack_msg, 5, message_length);

        func_retval = message_length;

    }

    return func_retval;
}




