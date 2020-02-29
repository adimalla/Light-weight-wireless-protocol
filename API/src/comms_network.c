/**
 ******************************************************************************
 * @file    comms_network.c
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
#include <stdlib.h>

#include "comms_network.h"






/******************************************************************************/
/*                                                                            */
/*                   Data Structures for wireless network                     */
/*                                                                            */
/******************************************************************************/

#define SYNC_HEADER_SIZE 6

/* Sync Message structure */
struct _sync_packet
{
    char         preamble[COMMS_PREAMBLE_LENTH];  /*!< Message preamble           */
    net_header_t fixed_header;                    /*!< Network header             */
    uint16_t     network_id;                      /*!< Network ID                 */
    uint8_t      message_slot_number;             /*!< Message slot number        */
    uint16_t     slot_time;                       /*!< Time interval of each slot */
    uint8_t      access_slot;                     /*!< Server Access number       */
    uint8_t      payload;                         /*!< Message payload            */
};


/* Network api error codes */
typedef enum _network_api_error_codes
{

    COMMS_SEND_ERROR        = -1,
    COMMS_RECV_ERROR        = -2,
    COMMS_SETTIMER_ERROR    = -3,
    COMMS_RESETTIMER_ERROR  = -4,
    COMMS_CLRINTRUPT_ERROR  = -5,
    COMMS_REQTIMEOUT_ERROR  = -6,
    COMMS_RESPTIMEOUT_ERROR = -7,
    COMMS_SYNCSTATUS_ERROR  = -8,
    COMMS_SENDSTATUS_ERROR  = -9,
    COMMS_RECVSTATUS_ERROR  = -10,
    COMMS_CLRSTATUS_ERROR   = -11,
    COMMS_GETSYNC_ERROR     = -12,
    COMMS_NETSTATUS_ERROR   = -13,

}net_api_retval_t;




/******************************************************************************/
/*                                                                            */
/*                              Private Functions                             */
/*                                                                            */
/******************************************************************************/



/* Implementation of glibc reentrant strtok Copyright (C) 1991-2019 GNU C Library */

static char *api_strtok_r (char *s, const char *delim, char **save_ptr)
{
    char *end;
    if (s == NULL)
        s = *save_ptr;
    if (*s == '\0')
    {
        *save_ptr = s;
        return NULL;
    }
    /* Scan leading delimiters.  */
    s += strspn (s, delim);
    if (*s == '\0')
    {
        *save_ptr = s;
        return NULL;
    }
    /* Find the end of the token.  */
    end = s + strcspn (s, delim);
    if (*end == '\0')
    {
        *save_ptr = end;
        return s;
    }
    /* Terminate the token and make *SAVE_PTR point past it.  */
    *end = '\0';
    *save_ptr = end + 1;
    return s;
}


/*
 * Copyright 1988-90 by Robert B. Stout dba MicroFirm,
 * Released to public domain, 1991,
 * https://people.cs.umu.se/isak/snippets/ltoa.c.
 */
#define API_LTOA_BUFSIZE (sizeof(long) * 8 + 1)

char *api_ltoa(long N, char *str, int base)
{
    register int i = 2;
    long uarg;
    char *tail, *head = str, buf[API_LTOA_BUFSIZE];

    if (36 < base || 2 > base)
        base = 10;                    /* can only use 0-9, A-Z        */
    tail = &buf[API_LTOA_BUFSIZE - 1];           /* last character position      */
    *tail-- = '\0';

    if (10 == base && N < 0L)
    {
        *head++ = '-';
        uarg    = -N;
    }
    else  uarg = N;

    if (uarg)
    {
        for (i = 1; uarg; ++i)
        {
            register ldiv_t r;

            r       = ldiv(uarg, base);
            *tail-- = (char)(r.rem + ((9L < r.rem) ?
                    ('A' - 10L) : '0'));
            uarg    = r.quot;
        }
    }
    else  *tail-- = '0';

    memcpy(head, ++tail, i);
    return str;
}




/********************************************************
 * @brief  static function to set mac address
 * @param  *device_mac  : device mac address (Hex)
 * @param  *mac_address : mac address (string)
 * @retval int8_t       : Error = -1, Success = 0
 ********************************************************/
static int8_t set_mac_address(char *device_mac, char *mac_address)
{
    int8_t func_retval = 0;

    char mac_address_copy[18] = {0};  /*!< Copy variable    */
    char *rest_ptr;                   /*!< Tracking pointer */
    char *token;                      /*!< token            */


    /* Copy mac address to copy variable for null termination (required for string function) */
    strncpy(mac_address_copy, mac_address, 18);  /* Size of mac address entered as string with null at index 18 */

    uint8_t index = 0;

    if(mac_address_copy[17] != 0)
    {
        func_retval = -1;
    }
    else
    {
        rest_ptr = mac_address_copy;

        /* strtok_r function for non glibc compliant code */
        while( (token = api_strtok_r(rest_ptr, ":", &rest_ptr)) )
        {
            /* Convert to hex */
            device_mac[index] = strtol(token, NULL, 16);

            index++;
        }

    }


    return func_retval;
}




/******************************************************************************/
/*                                                                            */
/*                         Weak Linked Functions                              */
/*                                                                            */
/******************************************************************************/


/* Send/Receive function operations */
__attribute__((weak)) int8_t send_message(char *message_buffer, uint8_t length)
{

    return 0;
}


__attribute__((weak)) int8_t recv_message(char *message_buffer, uint8_t length)
{

    return 0;
}

/* Transmit timer and receive interrupt operations */
__attribute__((weak)) int8_t set_timer(uint16_t device_slot_time, uint8_t device_slot_number)
{

    return 0;
}


__attribute__((weak)) int8_t reset_timer(void)
{

    return 0;
}


__attribute__((weak)) int8_t clear_receive_interrupt(void)
{

    return 0;
}



/* Timeout operations */
__attribute__((weak)) int8_t request_timeout(uint8_t timeout_seconds)
{

    return 0;
}


__attribute__((weak)) int8_t response_timeout(uint8_t timeout_seconds)
{

    return 0;
}



/* Network activity status operations */
__attribute__((weak)) int8_t sync_status(void)
{

    return 0;
}


__attribute__((weak)) int8_t send_status(void)
{

    return 0;
}



__attribute__((weak)) int8_t recv_status(void)
{

    return 0;
}


__attribute__((weak)) int8_t network_connected_status(void)
{

    return 0;
}



__attribute__((weak)) int8_t clear_status(void)
{

    return 0;
}


/* Network debug print operations */

__attribute__((weak)) int8_t debug_print_joinreq(char *debug_message)
{

    return 0;
}


__attribute__((weak)) int8_t debug_print_joinresp(char *debug_message)
{

    return 0;
}


__attribute__((weak)) int8_t debug_print_status(char *debug_message)
{

    return 0;
}


__attribute__((weak)) int8_t debug_print_contrl(char *debug_message)
{

    return 0;
}


__attribute__((weak)) int8_t debug_print_statusack(char *debug_message)
{

    return 0;
}



/******************************************************************************/
/*                                                                            */
/*                    Network Structure API Functions                         */
/*                                                                            */
/******************************************************************************/



/**************************************************************************************
 * @brief  Constructor function to create network access handle object
 * @param  *network_operations_t : reference to network operations handle
 * @retval access_control_t      : error: NULL, success: address of the created object
 **************************************************************************************/
access_control_t* create_network_handle(network_operations_t *network_ops)
{
    static access_control_t network;

    network.network_commands = network_ops;

    /* Configure weak implementations */

    /* Set send receive default callbacks */
    if(network_ops->send_message == NULL)
        network_ops->send_message = send_message;

    if(network_ops->recv_message == NULL)
        network_ops->recv_message = recv_message;


    /* Set send tx and rx timer and interrupt default callbacks */
    if(network_ops->set_tx_timer == NULL)
        network_ops->set_tx_timer = set_timer;

    if(network_ops->reset_tx_timer == NULL)
        network_ops->reset_tx_timer = reset_timer;

    if(network_ops->clear_recv_interrupt == NULL)
        network_ops->clear_recv_interrupt = clear_receive_interrupt;


    /* Set send timeout default callbacks */
    if(network_ops->request_timeout == NULL)
        network_ops->request_timeout  = request_timeout;

    if(network_ops->response_timeout == NULL)
        network_ops->response_timeout = response_timeout;


#if ACTIVITY_OPERATIONS

    /* Set activity default callbacks */
    if(network_ops->sync_activity_status == NULL)
        network_ops->sync_activity_status = sync_status;

    if(network_ops->send_activity_status == NULL)
        network_ops->send_activity_status = send_status;

    if(network_ops->recv_activity_status == NULL)
        network_ops->recv_activity_status = recv_status;

    if(network_ops->net_connected_status == NULL)
        network_ops->net_connected_status = network_connected_status;

    if(network_ops->clear_status == NULL)
        network_ops->clear_status = clear_status;

#endif

#if DEBUG_OPERATIONS

    /* Set network debug operations */
    if(network_ops->debug_print_joinreq == NULL)
        network_ops->debug_print_joinreq = debug_print_joinreq;

    if(network_ops->debug_print_joinresp == NULL)
        network_ops->debug_print_joinresp = debug_print_joinresp;

    if(network_ops->debug_print_status == NULL)
        network_ops->debug_print_status = debug_print_status;

    if(network_ops->debug_print_contrl == NULL)
        network_ops->debug_print_contrl = debug_print_contrl;

    if(network_ops->debug_print_statusack == NULL)
        network_ops->debug_print_statusack = debug_print_statusack;

#endif

    return &network;
}




/**************************************************************************************
 * @brief  Constructor function to create server device configure object
 * @param  *mac_address          : mac_address of the server device
 * @param  network_id            : network id of the server
 * @param  device_slot_time      : slot time interval
 * @param  total_slots           : no of existing slots at start
 * @retval device_config_t       : error: NULL, success: address of the created object
 **************************************************************************************/
device_config_t* create_server_device(char *mac_address, uint16_t network_id, uint16_t device_slot_time, uint8_t total_slots)
{

    static device_config_t server_device;

    if(device_slot_time == 0 || total_slots == 0 || network_id == 0 || mac_address == NULL)
    {
        return NULL;
    }
    else
    {
        /* Fixed Initializations */

        set_mac_address(server_device.device_mac, mac_address);

        server_device.device_network_id = network_id;

        server_device.device_slot_number = COMMS_SERVER_SLOTNUM;

        server_device.device_slot_time = device_slot_time;

        /* Values will change accordingly */

        /* Avoid reinitization of slots if called in ISR, (new object is created at every call) */
        if(total_slots > server_device.total_slots)
            server_device.total_slots = total_slots;

    }

    return &server_device;
}




/**************************************************************************************
 * @brief  Constructor function to create client device configure object
 * @param  *mac_address          : mac_address of the server device
 * @param  requested_total_slots : number of slots requested
 * @retval device_config_t       : error: NULL, success: address of the created object
 **************************************************************************************/
device_config_t* create_client_device(char *mac_address, uint8_t requested_total_slots)
{

    static device_config_t client_device;

    if(requested_total_slots == 0 || mac_address == NULL)
    {
        return NULL;
    }
    else
    {
        /* Set client mac address */
        set_mac_address(client_device.device_mac, mac_address);

        /* requested slots are the total slots held by the client device */
        client_device.total_slots = requested_total_slots;
    }

    return &client_device;
}




/***********************************************************************
 * @brief  Function to send message through network hardware
 * @param  *network         : reference to network handle structure
 * @param  *message_buffer  : message buffer to be send
 * @param  message_length   : message_length
 * @retval int8_t           : error: -1, success: length of message
 ***********************************************************************/
int8_t comms_send(access_control_t *network, char *message_buffer, uint8_t message_length)
{
    int8_t  func_retval   = 0;
    int8_t  send_retval   = 0;

    if(message_buffer == NULL || message_length == 0)
    {
        func_retval = COMMS_SEND_ERROR;
    }
    else
    {

#if 0   /* Future use , handle terminationfrom send function, requires change of all protocol functions */
        /* Add message terminator */
        strncpy(message_buffer + message_length, COMMS_MESSAGE_TERMINATOR, COMMS_TERMINATOR_LENGTH);
#endif

        send_retval = network->network_commands->send_message(message_buffer, message_length);
        if(send_retval < 0)
            func_retval = -1;
        else
            func_retval = message_length;
    }


    return func_retval;
}



/************************************************************************
 * @brief  Function to receive message through network hardware interrupt
 * @param  *network         : reference to network handle structure
 * @param  *message_buffer  : message buffer to be send
 * @param  *read_index      : index of buffer loop
 * @retval int8_t           : error: -2, success: length of message
 ************************************************************************/
int8_t comms_server_recv_it(access_control_t *network,comms_network_buffer_t *recv_buffer, uint8_t *read_index)
{

    int8_t func_retval =  0;

    uint8_t checksum = 0;

    /* Terminate the message on the message termination characters */
    if(recv_buffer->receive_message[*read_index] == 't' && recv_buffer->receive_message[*read_index - 1] == '\r')
    {

        network->network_commands->clear_recv_interrupt();

        network->packet_type = (void*)recv_buffer->receive_message;

        /* Validate checksum, Length of fixed header + preamble = 5 */
        checksum = comms_network_checksum((char*)recv_buffer->receive_message, 5, *read_index + 1);

        *read_index = 0;

        if(network->packet_type->fixed_header.message_checksum == checksum)
        {
            checksum = 0;

            /* Manage Network Access */
            if(network->packet_type->fixed_header.message_type < 10)
            {

                if(network->packet_type->fixed_header.message_type == COMMS_JOINREQ_MESSAGE)
                {
                    recv_buffer->flag_state = JOINREQ_FLAG;

                }

                if(network->packet_type->fixed_header.message_type == COMMS_STATUS_MESSAGE)
                {
                    recv_buffer->flag_state = STATUSMSG_FLAG;

                }

                /* Copy data to read buffer  */
                memset(recv_buffer->read_message, 0, sizeof(recv_buffer->read_message));
                memcpy(recv_buffer->read_message, recv_buffer->receive_message, network->packet_type->fixed_header.message_length + \
                       COMMS_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH);

            }
        }

        /* Clear receive buffer */
        memset(recv_buffer->receive_message, 0, sizeof(recv_buffer->receive_message));

        *read_index = 0;
    }
    else
    {
        (*read_index)++;

    }

    /* return length of message */
    func_retval = *read_index;

    return func_retval;
}




/************************************************************************
 * @brief  Function to receive message through network hardware interrupt
 * @param  *network         : reference to network handle structure
 * @param  *message_buffer  : message buffer to be send
 * @param  *read_index      : index of buffer loop
 * @retval int8_t           : error: -2, success: length of message
 ************************************************************************/
int8_t comms_client_recv_it(access_control_t *network,comms_network_buffer_t *recv_buffer, uint8_t *read_index)
{
    int8_t func_retval =  0;

    uint8_t checksum = 0;

    if(recv_buffer->receive_message[*read_index] == 't' && recv_buffer->receive_message[*read_index - 1] == '\r')
    {

        network->network_commands->clear_recv_interrupt();

        network->packet_type = (void*)recv_buffer->receive_message;

        /* Validate checksum */
        checksum = comms_network_checksum((char*)recv_buffer->receive_message, 5, network->packet_type->fixed_header.message_length + 5);

        *read_index = 0;

        if(network->packet_type->fixed_header.message_checksum == checksum)
        {

            checksum = 0;

            switch(network->packet_type->fixed_header.message_type)
            {

            case SYNC_FLAG:

                recv_buffer->flag_state = SYNC_FLAG;


                /* reset timer */
                network->network_commands->reset_tx_timer();


                /* Copy data to read buffer  */
                memset(recv_buffer->read_message, 0, sizeof(recv_buffer->read_message));
                memcpy(recv_buffer->read_message, recv_buffer->receive_message, network->packet_type->fixed_header.message_length + \
                       COMMS_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH);

                break;

            case JOINRESP_FLAG:

                recv_buffer->flag_state = JOINRESP_FLAG;

                /* Copy data to read buffer  */
                memset(recv_buffer->read_message, 0, sizeof(recv_buffer->read_message));
                memcpy(recv_buffer->read_message, recv_buffer->receive_message, network->packet_type->fixed_header.message_length + \
                       COMMS_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH);


                break;

            case CONTRLMSG_FLAG:

                recv_buffer->flag_state = CONTRLMSG_FLAG;

                /* Copy data to read buffer  */
                memset(recv_buffer->read_message, 0, sizeof(recv_buffer->read_message));
                memcpy(recv_buffer->read_message, recv_buffer->receive_message, network->packet_type->fixed_header.message_length + \
                       COMMS_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH);

                break;

            default:

                break;

            }

        }

        /* Clear receive buffer */
        memset(recv_buffer->receive_message, 0, sizeof(recv_buffer->receive_message));

    }
    else
    {
        (*read_index)++;
    }

    return func_retval;
}



/*********************************************************************
 * @brief  Function to set transmission timer for slotted network
 * @param  *network  : reference to network handle structure
 * @param  *device   : reference to the device configuration structure
 * @param  slot_type : type of network slot
 * @retval int8_t    : error: -3, success: 0
 *********************************************************************/
int8_t comms_network_set_timer(access_control_t *network, device_config_t *device, network_slot_t slot_type)
{
    int8_t func_retval      = 0;
    int8_t timer_api_retval = 0;

    if(device->device_slot_time == 0 || device->total_slots == 0)
    {
        func_retval = COMMS_SETTIMER_ERROR;
    }
    else
    {
        switch(slot_type)
        {

        case NET_BROADCAST_SLOT:

            timer_api_retval = network->network_commands->set_tx_timer(device->device_slot_time, NET_BROADCAST_SLOT);

            func_retval = 0;

            break;


        case NET_ACCESS_SLOT:

            timer_api_retval = network->network_commands->set_tx_timer(device->device_slot_time, NET_ACCESS_SLOT);

            func_retval = 0;

            break;


        case NET_SYNC_SLOT:

            /* sync slot is only a ranked slot as 1 by default and changes as per addition of devices */
            timer_api_retval = network->network_commands->set_tx_timer(device->device_slot_time, device->total_slots);

            func_retval = 0;


        case NET_CLIENT_ACCESS_SLOT:

            /* Client access lot number received from server */
            timer_api_retval = network->network_commands->set_tx_timer(device->device_slot_time, device->network_access_slot);

            func_retval = 0;

            break;


        case NET_CLIENT_SLOT:

            /* Client device slot received from server after successful join */
            timer_api_retval = network->network_commands->set_tx_timer(device->device_slot_time, device->device_slot_number);

            func_retval = 0;

            break;


        default:

            func_retval = COMMS_SETTIMER_ERROR;

            break;

        }

    }

    /* check timer api retval error */
    if( timer_api_retval < 0 )
    {
        func_retval = COMMS_SETTIMER_ERROR;
    }

    return func_retval;
}



/********************************************************
 * @brief  Function to calculate network message checksum
 * @param  data   : message data
 * @param  offset : starting offset for message data
 * @param  size   : length of message
 * @retval int8_t : checksum
 ********************************************************/
int8_t comms_network_checksum(char *data, uint8_t offset, uint8_t size)
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
/*                  Network Activity / Status Functions                       */
/*                                                                            */
/******************************************************************************/



/************************************************************
 * @brief  Function to enable sync activity status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -8, success = 0
 ************************************************************/
int8_t comms_sync_status(access_control_t *network)
{
    int8_t func_retval = 0;

    if(network == NULL)
    {
        func_retval = COMMS_SYNCSTATUS_ERROR;
    }
    else
    {
        network->network_commands->sync_activity_status();

        func_retval = 0;
    }

    return func_retval;
}




/************************************************************
 * @brief  Function to enable send activity status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -9, success = 0
 ************************************************************/
int8_t comms_send_status(access_control_t *network)
{
    int8_t func_retval = 0;

    if(network == NULL)
    {
        func_retval = COMMS_SENDSTATUS_ERROR;
    }
    else
    {
        network->network_commands->send_activity_status();

        func_retval = 0;
    }

    return func_retval;
}



/************************************************************
 * @brief  Function to enable receive activity status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -10, success = 0
 ************************************************************/
int8_t comms_recv_status(access_control_t *network)
{
    int8_t func_retval = 0;

    if(network == NULL)
    {
        func_retval = COMMS_RECVSTATUS_ERROR;
    }
    else
    {
        network->network_commands->recv_activity_status();

        func_retval = 0;
    }

    return func_retval;
}



/************************************************************
 * @brief  Function to enable clear activity status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -11, success = 0
 ************************************************************/
int8_t comms_clear_activity(access_control_t *network)
{
    int8_t func_retval = 0;

    if(network == NULL)
    {
        func_retval = COMMS_CLRSTATUS_ERROR;
    }
    else
    {
        network->network_commands->clear_status();

        func_retval = 0;
    }

    return func_retval;
}





/************************************************************
 * @brief  Function to enable network connected status
 * @param  *network  : reference to network handle structure
 * @retval int8_t    : error = -13, success = 0
 ************************************************************/
int8_t comms_net_connected_status(access_control_t *network)
{
    int8_t func_retval = 0;

    if(network == NULL)
    {
        func_retval = COMMS_NETSTATUS_ERROR;
    }
    else
    {
        network->network_commands->net_connected_status();

        func_retval = 0;
    }

    return func_retval;
}



/******************************************************************************/
/*                                                                            */
/*                      Network Debug functions                               */
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
int8_t comms_joinreq_debug_print(access_control_t *network, char *debug_message, uint8_t requested_slots)
{
    int8_t func_retval = 0;

    char requested_slots_ch[3] = {0};

    if(network == NULL || debug_message == NULL || requested_slots == 0 || requested_slots > 99)
    {
        func_retval = -14;
    }
    else
    {

        api_ltoa((long)requested_slots, requested_slots_ch, 1);

        network->network_commands->debug_print_joinreq(debug_message);

        network->network_commands->debug_print_joinreq(" ");

        network->network_commands->debug_print_joinreq("(SL:");
        network->network_commands->debug_print_joinreq(requested_slots_ch);
        network->network_commands->debug_print_joinreq(")");

        network->network_commands->debug_print_joinreq("\r\n");

    }

    return func_retval;
}



/************************************************************************
 * @brief  Function to print joinresp debug message
 *         Prints: " JOINRESP (ID:x) "
 * @param  *network              : reference to network handle structure
 * @param  debug_message         : debug message
 * @param  received_slot_id (ID) : received slot ID
 * @retval int8_t                : error = -15, success = 0
 ************************************************************************/
int8_t comms_joinresp_debug_print(access_control_t *network, char *debug_message, uint8_t received_slot_id)
{
    int8_t func_retval = 0;

    char received_slot_id_ch[4] = {0};

    if(network == NULL || debug_message == NULL || received_slot_id == 0 || received_slot_id > 999)
    {
        func_retval = -15;
    }
    else
    {

        api_ltoa(received_slot_id, received_slot_id_ch, 1);

        network->network_commands->debug_print_joinresp(debug_message);

        network->network_commands->debug_print_joinresp(" ");

        network->network_commands->debug_print_joinresp("(ID:");
        network->network_commands->debug_print_joinresp(received_slot_id_ch);
        network->network_commands->debug_print_joinresp(")");

        network->network_commands->debug_print_joinresp("\r\n");

    }

    return func_retval;
}




/***********************************************************************
 * @brief  Function to print status message debug
 *         Prints: " STATUS (DID:x LEN:x) DATA: 'message' "
 * @param  *network             : reference to network handle structure
 * @param  debug_message        : debug message
 * @param  destination_id (DID) : destination device ID
 * @param  payload_data         : payload message
 * @retval int8_t               : error = -16, success = 0
 ***********************************************************************/
int8_t comms_status_debug_print(access_control_t *network, char *debug_message, uint8_t destination_id, char *payload_data)
{

    int8_t func_retval = 0;

    char ltoa_buffer[4] = {0};

    uint8_t payload_length = strlen(payload_data);

    if(network == NULL || debug_message == NULL || destination_id == 0 || destination_id > 999)
    {
        func_retval = -16;
    }
    else
    {
        api_ltoa(destination_id, ltoa_buffer, 1);

        network->network_commands->debug_print_status(debug_message);

        network->network_commands->debug_print_status(" ");

        network->network_commands->debug_print_status("(DID:");
        network->network_commands->debug_print_status(ltoa_buffer);

        network->network_commands->debug_print_status(" ");

        network->network_commands->debug_print_status("LEN:");

        memset(ltoa_buffer, 0, sizeof(ltoa_buffer));

        api_ltoa(payload_length, ltoa_buffer, 1);

        network->network_commands->debug_print_status(ltoa_buffer);
        network->network_commands->debug_print_status(")");

        network->network_commands->debug_print_status(" ");

        network->network_commands->debug_print_status("DATA: ");
        network->network_commands->debug_print_status(payload_data);

        network->network_commands->debug_print_status("\r\n");

    }

    return func_retval;
}




/*******************************************************************
 * @brief  Function to print status message debug
 *         Prints: " CONTRL (SID:x LEN:x) DATA: 'message' "
 * @param  *network        : reference to network handle structure
 * @param  debug_message   : debug message
 * @param  source_id (DID) : source device ID
 * @param  payload_data    : payload message
 * @retval int8_t          : error = -16, success = 0
 *******************************************************************/
int8_t comms_contrl_debug_print(access_control_t *network, char *debug_message, uint8_t source_id, char *payload_data)
{

    int8_t func_retval = 0;

    char ltoa_buffer[4] = {0};

    uint8_t payload_length = strlen(payload_data);

    if(network == NULL || debug_message == NULL || source_id == 0 || source_id > 999)
    {
        func_retval = -17;
    }
    else
    {
        api_ltoa(source_id, ltoa_buffer, 1);

        network->network_commands->debug_print_contrl(debug_message);

        network->network_commands->debug_print_contrl(" ");

        network->network_commands->debug_print_contrl("(SID:");
        network->network_commands->debug_print_contrl(ltoa_buffer);

        network->network_commands->debug_print_contrl(" ");

        network->network_commands->debug_print_contrl("LEN:");

        memset(ltoa_buffer, 0, sizeof(ltoa_buffer));

        api_ltoa(payload_length, ltoa_buffer, 1);

        network->network_commands->debug_print_contrl(ltoa_buffer);
        network->network_commands->debug_print_contrl(")");

        network->network_commands->debug_print_contrl(" ");

        network->network_commands->debug_print_contrl("DATA: ");
        network->network_commands->debug_print_contrl(payload_data);

        network->network_commands->debug_print_contrl("\r\n");

    }

    return func_retval;
}





/******************************************************************************/
/*                                                                            */
/*                        API Functions (Client)                              */
/*                                                                            */
/******************************************************************************/



/*************************************************************************
 * @brief  Function to get sync message data
 * @param  *client_device   : reference to client device config structure
 * @param  *message_payload : message payload from sync message
 * @param  network          : network handle structure
 * @retval int8_t           : error: -12, success: 0
 *************************************************************************/
int8_t get_sync_data(device_config_t *client_device, char *message_payload ,access_control_t network)
{
    int8_t func_retval;

    if(client_device == NULL || network.sync_message == NULL || message_payload == NULL)
    {
        func_retval = COMMS_GETSYNC_ERROR;
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




/******************************************************************************/
/*                                                                            */
/*                        API Functions (Server)                              */
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
uint8_t comms_network_sync_message(access_control_t *network, uint16_t network_id, uint16_t slot_time,
                                   char *payload, uint16_t payload_length)
{
    uint8_t func_retval    = 0;
    uint8_t message_length = 0;
    uint8_t payload_index  = 0;
    char    *copy_payload  = 0;

    /* Check parameter error */
    if( network == NULL || slot_time > MAX_SLOT_TIME || payload == NULL)
    {
        func_retval = 0;
    }
    else
    {
        /* check length error */
        if(payload_length > COMMS_PAYLOAD_LENGTH - COMMS_TERMINATOR_LENGTH )
        {
            func_retval = 0;
        }
        else
        {
            network->sync_message->preamble[0] = (PREAMBLE_SYNC >> 8) & 0xFF;
            network->sync_message->preamble[1] = (PREAMBLE_SYNC >> 0) & 0xFF;

            network->sync_message->fixed_header.message_type = COMMS_SYNC_MESSAGE;

            network->sync_message->message_slot_number = COMMS_SERVER_SLOTNUM;

            network->sync_message->network_id = network_id;

            network->sync_message->access_slot = COMMS_ACCESS_SLOTNUM;

            network->sync_message->slot_time = slot_time;

            /* Add payload message */
            copy_payload = (void*)&network->sync_message->payload;

            memcpy(copy_payload, payload, payload_length);

            payload_index = payload_length;

            /* Add message terminator */
            strncpy(copy_payload + payload_index, COMMS_MESSAGE_TERMINATOR, COMMS_TERMINATOR_LENGTH);

            /* Calculate remaining length */
            network->sync_message->fixed_header.message_length = SYNC_HEADER_SIZE + payload_length + COMMS_TERMINATOR_LENGTH;

            message_length = network->sync_message->fixed_header.message_length + COMMS_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH;

            /* Calculate checksum Length of fixed header + PREAMBLE = 5 */
            network->sync_message->fixed_header.message_checksum = comms_network_checksum((char*)network->sync_message, 5 , message_length);

            func_retval = message_length;

        }
    }

    return func_retval;
}


