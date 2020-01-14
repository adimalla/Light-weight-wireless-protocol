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
typedef struct sync_packet sync_packet_t;


/* */
typedef struct device_config
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




typedef enum message_flags
{
    CLEAR_FLAG        = 0,
    SYNC_FLAG         = 1,
    JOINREQ_FLAG      = 2,
    JOINRESP_FLAG     = 3,
    STATUSMSG_FLAG    = 4,
    CONTRLMSG_FLAG    = 6,

}message_flags_t;




typedef struct application_flags
{
    uint8_t network_join_request      : 1;
    uint8_t network_join_response     : 1;
    uint8_t network_joined_state      : 1;
    uint8_t application_message_ready : 1;
    uint8_t network_message_ready     : 1;

}application_flags_t;




typedef struct comms_network_buffer
{
    char receive_message[COMMS_NET_MESSAGE_BUFFER_SIZE];
    char read_message[COMMS_NET_MESSAGE_BUFFER_SIZE];

    application_flags_t application_data;

    char application_message[COMMS_NET_MESSAGE_BUFFER_SIZE];
    char network_message[COMMS_NET_MESSAGE_BUFFER_SIZE];

    message_flags_t flag_state;


}comms_network_buffer_t;



typedef struct _network_operations
{
    int8_t (*send_message)(const char *message_buffer, uint8_t message_length);
    int8_t (*recv_message)(char *message_buffer, uint8_t message_length);

}network_operations_t;


/* */
typedef struct access_control
{
    sync_packet_t  *sync_message;  /*!< */

    network_operations_t *net_ops;

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
int8_t comms_network_checksum(char *data, uint8_t offset, uint8_t size);




/*************************************************************************
 * @brief  Function to get sync message data
 * @param  *client_device   : reference to client device config structure
 * @param  *message_payload : message payload from syn cmessage
 * @param  network          : network handle structure
 * @retval int8_t           : error -1, success: 0
 ************************************************************************/
int8_t get_sync_data(device_config_t *client_device, char *message_payload ,access_control_t network);




/***********************************************************************
 * @brief  Function to configure sync message
 * @param  *server_device   : reference to server device network handle
 * @param  network          : network handle structure
 * @param  *message_payload : message payload from sync message
 * @retval int8_t           : error: 0, success: length of message
 ***********************************************************************/
uint8_t comms_network_sync_message(access_control_t *server_device, uint16_t network_id, uint16_t slot_time, char *payload);



int8_t comms_send(access_control_t *network, const char *message_buffer, uint8_t message_length);



#endif /* COMMS_NETWORK_H_ */
