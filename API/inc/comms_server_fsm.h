/**
 ******************************************************************************
 * @file    comms_server_fsm.h
 * @author  Aditya Mall,
 * @brief   (6314) wireless network server state machine header file
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

#ifndef COMMS_SERVER_FSM_H_
#define COMMS_SERVER_FSM_H_




/*
 * Standard Header and API Header files
 */

#include <stdint.h>

#include "comms_network.h"
#include "comms_protocol.h"
#include "comms_server_db.h"





/******************************************************************************/
/*                                                                            */
/*                  Data Structures and Defines                               */
/*                                                                            */
/******************************************************************************/

typedef enum _comms_server_modes
{

    WI_LOCAL_SERVER   = 1,
    WI_GATEWAY_SERVER = 2


}comms_server_mode_t;




/******************************************************************************/
/*                                                                            */
/*                           API Prototypes                                   */
/*                                                                            */
/******************************************************************************/



/**************************************************************************
 * @brief  Server State Machine Start Function
 * @param  *wireless_network : reference to network access handle
 * @param  *server_device    : reference to device configuration structure
 * @param  *network_buffers  : reference to network buffers structure
 * @param  *client_devices   : reference to server client device DB table
 * @retval int8_t            : error = 0
 **************************************************************************/
int8_t comms_start_server(access_control_t *wireless_network, device_config_t *server_device,
                          comms_network_buffer_t *network_buffers, client_devices_t *client_devices,
                          comms_server_mode_t server_mode);






#endif /* COMMS_SERVER_FSM_H_ */
