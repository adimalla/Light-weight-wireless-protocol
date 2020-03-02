/*
 ******************************************************************************
 * @file    main.c
 * @author  Aditya Mall,
 * @brief
 *
 *  Info
 *
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



/******************************************************************************/
/*                                                                            */
/*              STANDARD LIBRARIES AND BOARD SPECIFIC HEADER FILES            */
/*                                                                            */
/******************************************************************************/

/*
 * Standard Header and API Header files
 */
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

/* Module Driver header file */
#include "console_driver.h"
#include "xbee_driver.h"
#include "cl_term.h"
#include "application_functions.h"

/* Protocol Driver header file */
#include "network_protocol_configs.h"
#include "comms_network.h"
#include "comms_protocol.h"
#include "comms_client_fsm.h"

/* Bare-metal header file */
#include "tm4c123gh6pm.h"


/******************************************************************************/
/*                                                                            */
/*                  Data Structures and Defines                               */
/*                                                                            */
/******************************************************************************/


/* network client app defines */

#define REQUESTED_SLOTS  1
#define DESTINATION_ID   5


/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/


 /* Initialize network buffer instance */
comms_network_buffer_t read_buffer;


/* Initialize Console Instance */
 cl_term_t *console;




/******************************************************************************/
/*                                                                            */
/*                           Function Implementations                         */
/*                                                                            */
/******************************************************************************/


 int8_t debug_print(char *message)
 {
     console_print(console, message);

     return 0;
 }



 int8_t clear_uart_recv_interrupt(void)
 {
     /* Clear UART interrupt */
     UART1_ICR_R |= (1 << 4);

     return 0;
 }



 /* Link Protocol functions */
 network_operations_t net_ops =
 {
  .reset_tx_timer       = reset_timer,
  .clear_recv_interrupt = clear_uart_recv_interrupt,
  .send_message         = xbee_send,
  .set_tx_timer         = set_tx_timer,
  .sync_activity_status = sync_led_status,
  .recv_activity_status = recv_led_status,
  .send_activity_status = send_led_status,
  .clear_status         = clear_led_status,
  .net_connected_status = network_joined_status,
  .net_debug_print      = debug_print,
 };



 /* Link console functions */
 console_ops_t serial_ops =
 {

  .open       = serial_open,
  .print_char = write_char,
  .read_char  = read_char,

 };


/******************************************************************************/
/*                                                                            */
/*                          Interrupt Routines                                */
/*                                                                            */
/******************************************************************************/



void gpioPortFIsr(void)
{

    GPIO_PORTF_ICR_R = 0x10;

    read_buffer.application_flags.network_join_request = 1;

}



/* Message RX ISR */
void uart1ISR(void)
{

    static uint8_t rx_index;

    access_control_t *network;

    network_operations_t net_ops =
    {
     .reset_tx_timer = reset_timer,
     .clear_recv_interrupt = clear_uart_recv_interrupt,
    };

    network = create_network_handle(&net_ops);

    char c = UART1_DR_R & 0xFF;

    read_buffer.receive_message[rx_index] = c;

    comms_client_recv_it(network, &read_buffer, &rx_index);

}


/* Message TX ISR */
void wTimer5Isr(void)
{

    WTIMER5_TAV_R = 0;
    WTIMER5_ICR_R = TIMER_ICR_TAMCINT;

    access_control_t *wireless_network;

    wireless_network = create_network_handle(&net_ops);

    device_config_t *client_device;

    client_device = create_client_device("10:20:14:15:16:17", REQUESTED_SLOTS);

    /* Init client state machine */
    comms_start_client(wireless_network, client_device, &read_buffer, 5);

}



/**
 * main.c
 */
int main(void)
{
    uint8_t loop         = 0;
    uint8_t input_length = 0;
    int8_t  retval       = 0;

    char text_buffer[NET_DATA_LENGTH] = {0};

    init_clock();

    init_board_io();

    init_xbee_comm();

    init_wide_timer_5();


    /* Open Console */
    console = console_open(&serial_ops, 115200, text_buffer, CONSOLE_STATIC);

    /* Clear Console Screen */
    console_print(console, CONSOLE_CLEAR_SCREEN);

    /* Enable Local Echo */
    console_print(console, CONSOLE_LOCAL_ECHO);

    /* Print Test output to console */
    console_print(console, "Device Test \n");

    loop = 1;

    while(loop)
    {
        /* Get input from user */
        input_length = console_get_string(console, MAX_INPUT_SIZE);

        retval = send_application_message(&read_buffer, text_buffer, input_length);

        if(retval <= 0)
        {
            console_print(console, "Not Connected to network \n");
        }


    }

    return 0;
}
