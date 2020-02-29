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

#include "tm4c123gh6pm.h"
#include "mcu_tm4c123gh6pm.h"
#include "gpio_tm4c123gh6pm.h"
#include "uart_tm4c123gh6pm.h"

#include "tiva_console.h"

#include "xbee_driver.h"

#include "network_protocol_configs.h"
#include "comms_network.h"
#include "comms_protocol.h"
#include "comms_client_fsm.h"



/******************************************************************************/
/*                                                                            */
/*                  Data Structures and Defines                               */
/*                                                                            */
/******************************************************************************/



//****************** Bit Banding defines for Pins *********************//      (for TM4C123GXL-TIVA-C LAUNCHPAD)

//Port F
#define PF1               (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
#define PF2               (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 2*4)))
#define PF3               (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))
#define PF4               (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 4*4)))


//***************************** Board Modules Pins *************************// (for TM4C123GXL-TIVA-C LAUNCHPAD)

#define ONBOARD_RED_LED           PF1
#define ONBOARD_BLUE_LED          PF2
#define ONBOARD_GREEN_LED         PF3
#define ONBOARD_PUSH_BUTTON       PF4



/* network client app defines */

#define REQUESTED_SLOTS  1
#define DESTINATION_ID   5


/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/


comms_network_buffer_t read_buffer;



/******************************************************************************/
/*                                                                            */
/*                         Functions Definitions                              */
/*                                                                            */
/******************************************************************************/



void init_clocks(void)
{
    //******************************************************* Clock Configs ******************************************************************//

    // Configure System clock as 40Mhz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (0x04 << SYSCTL_RCC_SYSDIV_S);

    // Enable GPIO port A, and F peripherals
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;

}



void init_board_io(void)
{

    //**************************************************** On Board Modules ******************************************************************//

    // Configure On boards RED, GREEN and BLUE led and Pushbutton Pins
    GPIO_PORTF_DEN_R |= (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);                  // Enable Digital
    GPIO_PORTF_DIR_R |= (1 << 1) | (1 << 2) | (1 << 3);                             // Enable as Output
    GPIO_PORTF_DIR_R &= ~(0x10);                                                    // Enable push button as Input
    GPIO_PORTF_PUR_R |= 0x10;                                                       // Enable internal pull-up for push button


    // GPIOF Interrupt
    GPIO_PORTF_IM_R |= 0x10;
    NVIC_EN0_R |= (1<<(INT_GPIOF-16));

}






void init_wide_timer_5(void)
{
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R5;                                       // turn-on timer
    WTIMER5_CTL_R &= ~TIMER_CTL_TAEN;                                                  // turn-off counter before reconfiguring
    WTIMER5_CFG_R  = 4;                                                                // configure as 32-bit counter (A only)

    WTIMER5_TAMR_R = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR | TIMER_TAMR_TAMIE;    // 1-Shot mode, Count up, Match interrupt Enable
    WTIMER5_TAMATCHR_R = (40000 * (5 * 1)) - 1;                                        // Initial Match load value before sync, get from EEPROM, given by server

    NVIC_EN3_R |= 1 << (INT_WTIMER5A-16-96);                                           // Enable interrupt in NVIC
    WTIMER5_IMR_R = TIMER_IMR_TAMIM;                                                   // Match Interrupt Enable

    WTIMER5_CTL_R |= TIMER_CTL_TAEN;                                                   // start
}





void gpioPortFIsr(void)
{

    GPIO_PORTF_ICR_R = 0x10;

    read_buffer.application_data.network_join_request = 1;

}






int8_t set_tx_timer(uint16_t device_slot_time, uint8_t device_slot_number)
{
    int8_t func_retval;

    if(device_slot_number == 0 || device_slot_time == 0)
    {
        func_retval = -1;
    }
    else
    {
        /* Calibrate Timer ISR to slot number received */
        WTIMER5_CTL_R &= ~TIMER_CTL_TAEN;

        while(!( SYSCTL->PRWTIMER & (1 << 5)) );

        WTIMER5_TAMATCHR_R  = ( 40000 * (device_slot_time * device_slot_number) ) - 1;

        while(!( SYSCTL->PRWTIMER & (1 << 5)) );

        WTIMER5_CTL_R |= TIMER_CTL_TAEN;

        WTIMER5_TAV_R  = 0;
    }

    return func_retval;
}



int8_t sync_led_status(void)
{

    ONBOARD_RED_LED    = 0;
    ONBOARD_BLUE_LED   = 0;
    ONBOARD_GREEN_LED ^= 1;

    return 0;
}


int8_t recv_led_status(void)
{

    ONBOARD_BLUE_LED = 0;
    ONBOARD_RED_LED ^= 1;

    return 0;
}

int8_t send_led_status(void)
{
    ONBOARD_RED_LED   = 0;
    ONBOARD_BLUE_LED ^= 1;

    return 0;
}



int8_t network_joined_status(void)
{
    ONBOARD_BLUE_LED  ^= 1;
    ONBOARD_GREEN_LED ^= 1;

    return 0;
}



int8_t clear_led_status(void)
{

    ONBOARD_RED_LED   = 0;
    ONBOARD_BLUE_LED  = 0;
    ONBOARD_GREEN_LED = 0;

    return 0;
}




int8_t reset_timer(void)
{

    /* Reset Send Timer */
    WTIMER5_CTL_R &= ~TIMER_CTL_TAEN;
    while(!( SYSCTL->PRWTIMER & (1 << 5)) );
    WTIMER5_TAV_R = 0;
    WTIMER5_CTL_R |= TIMER_CTL_TAEN;


    return 0;
}



int8_t clear_uart_recv_interrupt(void)
{
    /* Clear UART interrupt */
    UART1->ICR |= (1 << 4);

    return 0;
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

    char c = UART1->DR & 0xFF;

    read_buffer.receive_message[rx_index] = c;

    comms_client_recv_it(network, &read_buffer, &rx_index);

}



int8_t debug_print(char *message)
{
    uart_write(UART0, message, strlen(message));

    return 0;
}


/* Message TX ISR */
void wTimer5Isr(void)
{

    WTIMER5_TAV_R = 0;
    WTIMER5_ICR_R = TIMER_ICR_TAMCINT;

    access_control_t *wireless_network;

    network_operations_t net_ops =
    {
     .send_message         = xbee_send,
     .set_tx_timer         = set_tx_timer,
     .sync_activity_status = sync_led_status,
     .recv_activity_status = recv_led_status,
     .send_activity_status = send_led_status,
     .clear_status         = clear_led_status,
     .net_connected_status = network_joined_status,
     .debug_print_joinreq  = debug_print,
     .debug_print_joinresp = debug_print,
     .debug_print_status   = debug_print,
     .debug_print_contrl   = debug_print,

    };

    wireless_network = create_network_handle(&net_ops);

    device_config_t *client_device;

    client_device = create_client_device("20:20:14:15:16:17", REQUESTED_SLOTS);

    /* Init client state machine */
    comms_start_client(wireless_network, client_device, &read_buffer, 4);

}


/**
 * main.c
 */
int main(void)
{
    uint8_t loop = 0;

    console_t command_line;

    init_clocks();

    init_board_io();

    init_xbee_comm();

    init_wide_timer_5();


    memset(&command_line, 0, sizeof(command_line));

    /* Initialize Console */
    command_line.return_values = init_console(STDOUT);
    if(command_line.return_values == -1)
    {
        return 0;
    }

    /* Clear Console Screen */
    console_print(CONSOLE_CLEAR_SCREEN);

    /* Enable Local Echo */
    console_print(CONSOLE_LOCAL_ECHO);

    /* Print Test output to console */
    console_print("Device Test \n");


    loop = 1;


    while(loop)
    {
        /* Get input from user */


        command_line.return_values = console_getstring(&command_line, 20);

        if(command_line.return_values < 0)
        {
            console_print("String input greater than 20 \n");
            console_print(command_line.string_input);
            console_print("\n");
        }


        if(command_line.string_input[0] != '\0')
        {
            if(read_buffer.application_data.network_joined_state == 1)
            {

                /* Copy user input to comms buffer */
                memset(read_buffer.application_message, 0, sizeof(read_buffer.application_message));

                strncpy(read_buffer.application_message,command_line.string_input,20);

                read_buffer.application_data.application_message_ready = 1;
            }
            else
            {
                console_print("Not Connected to network \n");
            }
        }



    }

    return 0;
}
