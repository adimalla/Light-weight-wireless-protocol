/**
 ******************************************************************************
 * @file    comms_client_fsm.c
 * @author  Aditya Mall,
 * @brief   (6314) wireless network client state machine source file
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
#include "comms_client_fsm.h"


/******************************************************************************/
/*                                                                            */
/*                  Data Structures and Defines                               */
/*                                                                            */
/******************************************************************************/




/* Device Specific */
typedef enum client_state_machine_values
{
    DEV_INIT     = 1,
    DEV_SYNC     = 2,
    DEV_JOINREQ  = 3,
    DEV_JOINED   = 4

}client_fsm_states_t;




/******************************************************************************/
/*                                                                            */
/*                           API Functions                                    */
/*                                                                            */
/******************************************************************************/



/**************************************************************************
 * @brief  Client State Machine Start Function
 * @param  *wireless_network : reference to network access handle
 * @param  *server_device    : reference to device configuration structure
 * @param  *network_buffers  : reference to network buffers structure
 * @param  destination_id    : device id of the destination
 * @retval int8_t            : error = 0
 **************************************************************************/
int8_t comms_start_client(access_control_t *wireless_network, device_config_t *client_device,
                          comms_network_buffer_t *network_buffers, uint8_t destination_id)
{

    int8_t func_retval = 1;
    protocol_handle_t  client;

    char    sync_message_buff[20]        = {0};
    char    message_buffer[NET_MTU_SIZE] = {0};
    uint8_t message_length               = 0;

    static uint8_t debug_print_count = 0;

    static  client_fsm_states_t fsm_state = DEV_INIT;

    switch(fsm_state)
    {

    case DEV_INIT:

        fsm_state = DEV_SYNC;

        break;


    case DEV_SYNC:

        if(network_buffers->flag_state == SYNC_FLAG)
        {

            comms_sync_status(wireless_network);

            fsm_state = DEV_SYNC;

            if(network_buffers->application_flags.network_join_request == 1)
            {
                /* Get data from read buffer */
                wireless_network->sync_message = (void*)network_buffers->read_message;

                /* Get sync data (access slot and network id) */
                get_sync_data(client_device, sync_message_buff, *wireless_network);

                /* Clear read buffer after reading the message */
                memset(network_buffers->read_message, 0, sizeof(network_buffers->read_message));

                /* calibrate timer to access slot */
                comms_network_set_timer(wireless_network, client_device, NET_CLIENT_ACCESS_SLOT);


                fsm_state = DEV_JOINREQ;
            }

            network_buffers->flag_state = CLEAR_FLAG;
        }


        break;


    case DEV_JOINREQ:


        /* Send JOINREQ Message until JOINRESP message is not received */
        if(network_buffers->flag_state == SYNC_FLAG && network_buffers->flag_state != JOINRESP_FLAG && \
                network_buffers->application_flags.network_join_request == 1)
        {

            comms_send_status(wireless_network);

            client.joinrequest_msg = (void*)message_buffer;

            /* configure JOINREQ message options*/
            comms_joinreq_options(&client, 0, 1);

            /* configure JOINREQ message fields */
            message_length = comms_joinreq_message(&client, *client_device, 1);

            /* Send join request message */
            comms_send(wireless_network, (char*)client.joinrequest_msg, message_length);

            network_buffers->flag_state = CLEAR_FLAG;

            /* Print once */
            comms_joinreq_debug_print(wireless_network, "JOINREQ", client_device->total_slots);


#if JOINREQ_ONCE
            network_buffers->application_flags.network_join_request = 0;
#endif

        }


        /*Get JOINRESP Message data from WI network server*/
        if(network_buffers->flag_state == JOINRESP_FLAG)
        {

            client.joinresponse_msg = (void*)network_buffers->read_message;

            /* Get JOINRESP data */
            comms_get_joinresp_data(client_device, client);

            if(client_device->device_slot_number)
            {

                comms_clear_activity(wireless_network);

                /* Set network flag as joined */
                client_device->network_joined = 1;
                network_buffers->application_flags.network_joined_state = 1;

                /* Calibrate new slot time */
                comms_network_set_timer(wireless_network, client_device, NET_CLIENT_SLOT);

                /* Reset join request flag */
                network_buffers->application_flags.network_join_request = 0;

                /* Change state to joined */
                fsm_state = DEV_JOINED;

                /*Print JOINREQ debug message */
                comms_joinresp_debug_print(wireless_network, "JOINRESP", client_device->device_slot_number);

                /* Clear print only once for JOINREQ debug message */
                debug_print_count = 0;

            }

            memset(network_buffers->read_message, 0, sizeof(network_buffers->read_message));

            network_buffers->flag_state = CLEAR_FLAG;

        }


        break;


    case DEV_JOINED:

        /* Network joined status*/
        if(network_buffers->flag_state == SYNC_FLAG)
        {
            comms_net_connected_status(wireless_network);
        }


        /* Send Status message when app message is ready */
        if(network_buffers->flag_state == SYNC_FLAG && network_buffers->application_flags.application_message_ready == 1 )
        {
            /* Send STATUS Message when application message is available */
            comms_send_status(wireless_network);

            client.status_msg = (void*)message_buffer;

            /* Configure Status message */
            message_length = comms_status_message(&client, *client_device, destination_id,
                                                  network_buffers->application_message, network_buffers->app_message_length);

            /* Send Status Message */
            comms_send(wireless_network, (char*)client.status_msg, message_length);

            network_buffers->application_flags.application_message_ready = 0;

            comms_status_debug_print(wireless_network, "STATUS",destination_id, network_buffers->application_message);

        }


        /* Get CONTROL Message data*/
        if(network_buffers->flag_state == CONTRLMSG_FLAG)
        {

            client.contrl_msg = (void*)network_buffers->read_message;

            memset(network_buffers->network_message, 0, sizeof(network_buffers->network_message));

            /* read control message */
            message_length = comms_get_contrl_data(network_buffers->network_message, &network_buffers->source_id, client, \
                                                   client_device->device_network_id, client_device->device_slot_number);

            network_buffers->destination_id = client_device->device_slot_number;

            if(message_length)
            {
                network_buffers->application_flags.network_message_ready = 1;

                comms_recv_status(wireless_network);

                comms_contrl_debug_print(wireless_network, "CONTROL", network_buffers->source_id, network_buffers->network_message);

            }

            memset(network_buffers->read_message, 0, sizeof(network_buffers->read_message));

        }

        network_buffers->flag_state = CLEAR_FLAG;

        break;


    default:

        fsm_state = DEV_SYNC;

        break;

    }

    func_retval = 0;

    return func_retval;
}

