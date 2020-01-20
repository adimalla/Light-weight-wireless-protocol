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

    char    send_message_buffer[COMMS_MESSAGE_LENGTH] = {0};

    uint8_t client_requested_slots                    = 0;
    char    client_mac_address[COMMS_MACADDR_SIZE]    = {0};

    char    destination_mac_addr[COMMS_MACADDR_SIZE]  = {0};

    uint8_t message_length = 0;

    static int8_t  client_id                 = 0;    /*!< from device table   */
    static uint8_t destination_client_id     = 0;    /*!< from status message */
    static uint8_t source_client_id          = 0;
    static char    status_message_buffer[20] = {0};

    static int8_t device_found = 0;

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

        /* Clear Activity Status */
        comms_clear_activity(wireless_network);

        fsm_state = network_buffers->flag_state;
        if(fsm_state == 0)
        {
            fsm_state = SYNC_STATE;

        }

        /* clear flag */
        network_buffers->flag_state = CLEAR_FLAG;

        break;



    case SYNC_STATE:

        /* Activity, Status LED function for sync message, access via user callback */
        comms_sync_status(wireless_network);

        wireless_network->sync_message = (void*)send_message_buffer;

        message_length = comms_network_sync_message(wireless_network, server_device->device_network_id, server_device->device_slot_time, "sync");

        comms_send(wireless_network, (char*)wireless_network->sync_message, message_length);

        fsm_state = MSG_READ_STATE;

        /* Not implemented */
        if(contrl_flag)
        {
            /* Set timer */

            fsm_state = CONTROLMSG_STATE;

        }

        break;


    case JOINREQ_STATE:

        /* Activity, Status LED function for receiving messages, access via user callback */
        comms_recv_status(wireless_network);

        server.joinrequest_msg = (void*)network_buffers->read_message;

        api_retval = comms_get_joinreq_data(client_mac_address, &client_requested_slots, server,
                                            *server_device, network_buffers->application_data.network_join_response);

        if(api_retval)
        {

            table_values = update_server_device_table(client_devices, client_mac_address, client_requested_slots, server_device);

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
        read_client_table(client_devices, destination_mac_addr, &client_id, table_values.table_index);

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

                    fsm_state = CONTROLMSG_STATE;

                }
                else
                {
                    /* search table for */
                    device_found = find_client_device(client_devices, &destination_client_id, client_mac_address, 1);

                    /* Check device found condition */
                    if(device_found == 0)
                        destination_client_id = device_found;

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


    case STATUSACK_STATE: /* Not implemented / tested */

        memset(send_message_buffer, 0, sizeof(send_message_buffer));

        server.statusack_msg = (void*)send_message_buffer;

        message_length = comms_statusack_message(&server, *server_device, client_id, destination_client_id);

        /* Call Send function */

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

        /* set flags and parameters to init values */
        contrl_flag           = 0;
        device_found          = 0;
        source_client_id      = 0;
        destination_client_id = 0;
        memset(status_message_buffer, 0, sizeof(status_message_buffer));

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

