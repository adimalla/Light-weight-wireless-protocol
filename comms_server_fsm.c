/**
 ******************************************************************************
 * @file    comms_server_fsm.c
 * @author  Aditya Mall,
 * @brief   (6314) wireless network server state machine source file
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
#include "comms_server_fsm.h"





/******************************************************************************/
/*                                                                            */
/*                  Data Structures and Defines                               */
/*                                                                            */
/******************************************************************************/


typedef enum fsm_state_values
{
    START_STATE       = 15,
    MSG_READ_STATE    = 10,
    SYNC_STATE         = 1,
    JOINREQ_STATE      = 2,
    JOINRESP_STATE     = 3,
    STATUSMSG_STATE    = 4,
    STATUSACK_STATE    = 5,
    CONTROLMSG_STATE   = 6,
    EVENTMSG_STATE     = 7,
    TIMEOUT_STATE      = 8,
    EXIT_STATE         = 9

}fsm_states_t;




/******************************************************************************/
/*                                                                            */
/*                              Private Functions                             */
/*                                                                            */
/******************************************************************************/


static int8_t get_client_mac_address(char *client_mac_address, protocol_handle_t server)
{
    int8_t func_retval = 0;

    strncpy(client_mac_address, server.joinrequest_msg->source_mac, 6);

    return func_retval;
}




/******************************************************************************/
/*                                                                            */
/*                           API Functions                                    */
/*                                                                            */
/******************************************************************************/




/**************************************************************************
 * @brief  Server State Machine Start Function
 * @param  *wireless_network : reference to network access handle
 * @param  *server_device    : reference to device configuration structure
 * @param  *network_buffers  : reference to network buffers structure
 * @param  *client_devices   : reference to server client device DB table
 * @param  server_mode       : sever device operation mode
 * @retval int8_t            : error = 0
 **************************************************************************/
int8_t comms_start_server(access_control_t *wireless_network, device_config_t *server_device, comms_network_buffer_t *network_buffers,
                          client_devices_t *client_devices, comms_server_mode_t server_mode)
{

    int8_t func_retval = 1;
    int8_t api_retval  = 0;

    protocol_handle_t server;

    char    send_message_buffer[32]                        = {0};
    char    destination_mac_addr[COMMS_NET_MACADDR_LENGTH] = {0};

    uint8_t message_length = 0;

    static int8_t  client_id                 = 0;    /*!< from device table   */
    static uint8_t destination_client_id     = 0;    /*!< from status message */
    static uint8_t source_client_id          = 0;
    static char    status_message_buffer[20] = {0};

    static uint8_t contrl_flag = 0;

    /* Device DB related declarations */
    static table_retval_t  table_values;


    static int8_t fsm_state = START_STATE;

    switch(fsm_state)
    {

    case START_STATE:

        /* Set timer */
        comms_network_set_timer(wireless_network, server_device, NET_SYNC_SLOT);

        fsm_state = SYNC_STATE;

        break;


    case MSG_READ_STATE:


        comms_clear_activity(wireless_network);

        fsm_state = network_buffers->flag_state;
        if(fsm_state == 0)
        {
            fsm_state = SYNC_STATE;

        }
        else
        {
            /* change state according to message type */
            server.packet_type = (void*)network_buffers->read_message;

            fsm_state = server.packet_type->fixed_header.message_type;

        }

        network_buffers->flag_state = CLEAR_FLAG;

        break;



    case SYNC_STATE:

        /* Activity, Status LED function for sync message, access via user callback */
        comms_sync_status(wireless_network);

        wireless_network->sync_message = (void*)send_message_buffer;

        message_length = comms_network_sync_message(wireless_network, server_device->device_network_id, server_device->device_slot_time, "sync");

        comms_send(wireless_network, (char*)wireless_network->sync_message, message_length);

        fsm_state = MSG_READ_STATE;

        if(contrl_flag)
        {
            //set_tx_timer(COMMS_SERVER_SLOT_TIME, COMMS_BROADCAST_SLOTNUM);

            fsm_state = CONTROLMSG_STATE;

        }

        break;


    case JOINREQ_STATE:

        /* Activity, Status LED function for receiving messages, access via user callback */
        comms_recv_status(wireless_network);

        server.joinrequest_msg = (void*)network_buffers->read_message;

        api_retval = comms_get_joinreq_data(server, *server_device, network_buffers->application_data.network_join_response);

        if(api_retval)
        {
            table_values = update_client_table(client_devices, &server, server_device);

            network_buffers->application_data.network_join_response = 0;

            switch(server_mode)
            {

            case WI_LOCAL_SERVER:

                /* Set timer to broadcast slot */
                comms_network_set_timer(wireless_network, server_device, NET_BROADCAST_SLOT);

                fsm_state = JOINRESP_STATE;

                break;

            case WI_GATEWAY_SERVER:

                /* Set network message flag read and network message in gateway mode */
                network_buffers->application_data.network_message_ready = 1;

                break;

            }

        }
        else
        {
            network_buffers->application_data.network_join_response = 0;

            fsm_state = SYNC_STATE;
        }


        network_buffers->flag_state = CLEAR_FLAG;

        memset(network_buffers->read_message, 0, sizeof(network_buffers->read_message));


        break;



    case JOINRESP_STATE:

        /* Activity, Status LED function for sending messages, access via user callback */
        comms_send_status(wireless_network);

        /* send join response at broadcast slot and reset to updated slot */

        memset(send_message_buffer, 0, sizeof(send_message_buffer));

        server.joinresponse_msg = (void*)send_message_buffer;

        /* Get client data from the device table */
        read_client_table(destination_mac_addr, &client_id, client_devices, table_values.table_index);

        /* Set join response message type */
        comms_set_joinresp_message_status(&server, table_values.table_retval);

        /* Configure JOINRESP message */
        message_length = comms_joinresp_message(&server, *server_device, destination_mac_addr, client_id);

        /* Send JOINRESP message */
        comms_send(wireless_network, (char*)server.joinresponse_msg, message_length);

        /* update timer to accommodate new slot */
        comms_network_set_timer(wireless_network, server_device, NET_SYNC_SLOT);

        fsm_state = SYNC_STATE;

        break;



    case STATUSMSG_STATE:


        /* Activity, Status LED function for receiving messages, access via user callback */
        comms_recv_status(wireless_network);


        /* Read Status message and send control message to the destination device */

        server.status_msg = (void*)network_buffers->read_message;

        /* Check network id */
        if(server.status_msg->network_id == server_device->device_network_id)
        {
            /* get destination client and payload from status */
            comms_get_status_message(status_message_buffer, &source_client_id, &destination_client_id, server);

            if(server_mode == WI_LOCAL_SERVER)
            {
                /* handle server message condition (not done) */
                if(destination_client_id == 1)
                {
                    /* calibrate timer to broadcast message */
                    //set_tx_timer(COMMS_SERVER_SLOT_TIME, COMMS_BROADCAST_SLOTNUM);

                    fsm_state = CONTROLMSG_STATE;

                }
                else
                {
                    /* search table for */
                    read_client_table(destination_mac_addr, &client_id, client_devices, destination_client_id - 4);

                    /* Set timer to broadcast slot */
                    comms_network_set_timer(wireless_network, server_device, NET_BROADCAST_SLOT);

                    fsm_state = CONTROLMSG_STATE;

                }
            }

        }
        else
        {
            fsm_state = SYNC_STATE;
        }

        memset(network_buffers->read_message, 0, sizeof(network_buffers->read_message));

        network_buffers->flag_state = CLEAR_FLAG;


        break;


    case STATUSACK_STATE:

        memset(send_message_buffer, 0, sizeof(send_message_buffer));

        server.statusack_msg = (void*)send_message_buffer;

        message_length = comms_statusack_message(&server, *server_device, client_id, destination_client_id);

        //uart_write(UART1, (char*)server.statusack_msg, message_length);

        contrl_flag = 1;

        fsm_state = MSG_READ_STATE;

        break;



    case CONTROLMSG_STATE:


        /* Activity, Status LED function for sending messages, access via user callback */
        comms_send_status(wireless_network);

        memset(send_message_buffer, 0, sizeof(send_message_buffer));

        server.contrl_msg = (void*)send_message_buffer;

        /* echo condition (compare ID gotten from status message with ID gotten from device table )*/

        message_length = comms_control_message(&server, *server_device, source_client_id, destination_client_id, status_message_buffer);

        comms_send(wireless_network, (char*)server.contrl_msg, message_length);

        /* set timer to sync slot after sending CONTRL message */
        comms_network_set_timer(wireless_network, server_device, NET_SYNC_SLOT);

        memset(status_message_buffer, 0, sizeof(status_message_buffer));

        contrl_flag = 0;

        fsm_state = SYNC_STATE;

        break;


    default:

        fsm_state = MSG_READ_STATE;

        break;

    }

    /* Error condition if state machine breaks */
    func_retval = 0;

    return func_retval;

}

